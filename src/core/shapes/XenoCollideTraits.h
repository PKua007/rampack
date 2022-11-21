//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_XENOCOLLIDETRAITS_H
#define RAMPACK_XENOCOLLIDETRAITS_H

#include <map>
#include <utility>
#include <sstream>
#include <optional>

#include "core/ShapeTraits.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "geometry/xenocollide/XenoCollide.h"
#include "XCWolframShapePrinter.h"
#include "XCObjShapePrinter.h"
#include "geometry/Polyhedron.h"
#include "utils/Assertions.h"


/**
 * @brief A shape with XenoCollide intersection test.
 * @details <p> The class implements all ShapeTraits methods, so that the deriving class @a ConcreteCollideTraits has
 * to only provide @a CollideGeometry objects (conforming to XenoCollide template parameter), shape axes, origin, volume
 * and optionally named points (see ShapeGeometry::getNamedPoint). It supports multiple interaction centres, so one can
 * combine many XenoCollide shapes into a one concave object. Derived class specifies interaction centers by overriding
 * Interaction::getInteractionCentres() method on its own. It must then support as many @a CollideGeometry objects
 * (see @a XenoCollideTraits::ConcreteCollideTraits template parameter).
 *
 * <p> Assuming the deriving class is called @a MyShape it should derive from XenoCollideTraits like this (CRTP idiom):
 * @code
 * class MyShape : public XenoCollideTraits<MyShape> {
 *     ...
 * }
 * @endcode
 * @tparam ConcreteCollideTraits a deriving class (CRTP idiom). It is requires to have a method with signature
 * @code
 * // Returns CollideGeometry object (arbitrary class but conforming to XenoCollide template parameter) for an
 * // interaction center with index idx. If ConcreteCollideTraits has only a single interaction center, the argument
 * // should be ignored, but signature must not be altered
 * const CollideGeometry &ConcreteCollideTraits::getCollideGeometry(std::size_t idx) const
 * @endcode
 */
template<typename ConcreteCollideTraits>
class XenoCollideTraits : public ShapeTraits, public Interaction, public ShapeGeometry {
private:
    Vector<3> primaryAxis;
    Vector<3> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};

    mutable std::optional<double> rangeRadius;
    mutable std::unique_ptr<ShapePrinter> wolframPrinter;
    mutable std::unique_ptr<ShapePrinter> objPrinter;

    template<typename Printer>
    std::unique_ptr<Printer> createPrinter() const {
        auto centers = this->getInteractionCentres();
        if (centers.empty())
            centers.push_back({0, 0, 0});

        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        using Geometry = decltype(thisConcreteTraits.getCollideGeometry(0));

        std::vector<PolymorphicXCAdapter<Geometry>> geometries;
        std::vector<const AbstractXCGeometry *> geometryPointers;
        geometries.reserve(centers.size());
        geometryPointers.reserve(centers.size());
        for (std::size_t i{}; i < centers.size(); i++) {
            geometries.emplace_back(thisConcreteTraits.getCollideGeometry(i));
            geometryPointers.push_back(&geometries.back());
        }

        return std::make_unique<Printer>(geometryPointers, centers, MESH_SUBDIVISIONS);
    }

public:
    /** @brief The number of sphere subdivisions when printing the shape (see XCPrinter::XCPrintes @a subdivision
     * parameter) */
    static constexpr std::size_t MESH_SUBDIVISIONS = 3;

    /**
     * @brief Programmes the shape with given parameters.
     * @param primaryAxis primary axis of the shape (see ShapeGeometry::getPrimaryAxis())
     * @param secondaryAxis secondary axis of the shape (see ShapeGeometry::getSecondaryAxis())
     * @param geometricOrigin geometric origin of the shape (see ShapeGeometry::getGeometricOrigin())
     * @param volume volume of the shape (it has to be computed manually, since there is no exact general method)
     * @param customNamedPoints optional list of named points (see ShapeGeometry::getNamedPoint())
     */
    XenoCollideTraits(const Vector<3> &primaryAxis, const Vector<3> &secondaryAxis, const Vector<3> &geometricOrigin,
                      double volume, const ShapeGeometry::NamedPoints &customNamedPoints = {})
            : primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis}, geometricOrigin{geometricOrigin}, volume{volume}
    {
        this->registerNamedPoints(customNamedPoints);
    }

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

    [[nodiscard]] const ShapePrinter &getPrinter(const std::string &format) const override {
        if (format == "wolfram") {
            if (this->wolframPrinter == nullptr)
                this->wolframPrinter = this->template createPrinter<XCWolframShapePrinter>();
            return *this->wolframPrinter;
        } else if (format == "obj") {
            if (this->objPrinter == nullptr)
                this->objPrinter = this->template createPrinter<XCObjShapePrinter>();
            return *this->objPrinter;
        } else {
            throw NoSuchShapePrinterException("XenoCollideTraits: unknown printer format: " + format);
        }
    }

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const final {
        return shape.getOrientation() * this->primaryAxis;
    }

    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const final {
        return shape.getOrientation() * this->secondaryAxis;
    }

    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const final {
        return shape.getOrientation() * this->geometricOrigin;
    }

    [[nodiscard]] double getVolume() const final { return this->volume; }

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasWallPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool isConvex() const override { return true; }

    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override
    {
        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        const auto &collideGeometry1 = thisConcreteTraits.getCollideGeometry(idx1);
        const auto &collideGeometry2 = thisConcreteTraits.getCollideGeometry(idx2);
        using XCGeometry = decltype(collideGeometry1);
        double rCircumsphere = collideGeometry1.getCircumsphereRadius() + collideGeometry2.getCircumsphereRadius();
        double rInsphere = collideGeometry1.getInsphereRadius() + collideGeometry2.getInsphereRadius();

        Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
        double dist2 = (pos2bc - pos1).norm2();
        if (dist2 > rCircumsphere*rCircumsphere)
            return false;
        if (dist2 < rInsphere*rInsphere)
            return true;

        return XenoCollide<XCGeometry>::Intersect(collideGeometry1, orientation1, pos1,
                                                  collideGeometry2, orientation2, pos2bc,
                                                  1.0e-12);
    }

    [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation,
                                       [[maybe_unused]] std::size_t idx, const Vector<3> &wallOrigin,
                                       const Vector<3> &wallVector) const override
    {
        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        const auto &collideGeometry = thisConcreteTraits.getCollideGeometry(idx);

        Vector<3> normalVector = (orientation.transpose())*(wallVector);
        Vector<3> supportPoint = collideGeometry.getSupportPoint(-normalVector);
        Vector<3> origin = (orientation.transpose())*(wallOrigin - pos);
        // minus sign because we count distance along -normalVector
        double distanceSupport = -supportPoint * normalVector;
        double distanceWall = -origin*normalVector;
        if (distanceWall > distanceSupport)
            return false;
        return true;
    }

    [[nodiscard]] double getRangeRadius() const override {
        if (this->rangeRadius.has_value())
            return *this->rangeRadius;

        std::size_t numCenters = std::max(this->getInteractionCentres().size(), 1ul);
        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        double maxRadius{};
        for (std::size_t i{}; i < numCenters; i++) {
            double newRadius = thisConcreteTraits.getCollideGeometry(i).getCircumsphereRadius();
            if (newRadius > maxRadius)
                maxRadius = newRadius;
        }
        this->rangeRadius = 2*maxRadius;
        return *this->rangeRadius;
    }
};


#endif //RAMPACK_XENOCOLLIDETRAITS_H
