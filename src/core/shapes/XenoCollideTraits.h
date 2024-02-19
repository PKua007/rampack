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
#include "utils/Exceptions.h"
#include "OptionalAxis.h"


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
class XenoCollideTraits : public ShapeTraits, public Interaction, public ShapeGeometry, public ShapeDataManager {
private:
    std::optional<Vector<3>> primaryAxis;
    std::optional<Vector<3>> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};

    mutable std::optional<double> rangeRadius;

    template<typename Printer>
    std::shared_ptr<Printer> createPrinter(std::size_t meshSubdivisions) const {
        // TODO: fix nullptr
        auto centers = this->getInteractionCentres(nullptr);
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

        return std::make_shared<Printer>(geometryPointers, centers, meshSubdivisions);
    }

public:
    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    /**
     * @brief Programmes the shape with given parameters.
     * @param primaryAxis primary axis of the shape (see ShapeGeometry::getPrimaryAxis())
     * @param secondaryAxis secondary axis of the shape (see ShapeGeometry::getSecondaryAxis())
     * @param geometricOrigin geometric origin of the shape (see ShapeGeometry::getGeometricOrigin())
     * @param volume volume of the shape (it has to be computed manually, since there is no exact general method)
     * @param customNamedPoints optional list of named points (see ShapeGeometry::getNamedPoint())
     */
    XenoCollideTraits(OptionalAxis primaryAxis, OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin,
                      double volume, const ShapeGeometry::StaticNamedPoints &customNamedPoints = {})
            : primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis}, geometricOrigin{geometricOrigin}, volume{volume}
    {
        this->registerStaticNamedPoints(customNamedPoints);
    }

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }
    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }

    /**
     * @brief Returns ShapePrinter for a given @a format.
     * @details The following formats are supported:
     * <ol>
     *     <li> `wolfram` - Wolfram Mathematica shape (it accepts @a mesh_divisions parameter, default: 3)
     *     <li> `obj` - Wavefront OBJ triangle mesh (it accepts @a mesh_divisions parameter, default: 3)
     * </ol>
     */
    [[nodiscard]] std::shared_ptr<const ShapePrinter>
    getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const override {
        std::size_t meshSubdivisions = DEFAULT_MESH_SUBDIVISIONS;
        if (params.find("mesh_divisions") != params.end()) {
            meshSubdivisions = std::stoul(params.at("mesh_divisions"));
            Expects(meshSubdivisions >= 1);
        }

        if (format == "wolfram")
            return this->template createPrinter<XCWolframShapePrinter>(meshSubdivisions);
        else if (format == "obj")
            return this->template createPrinter<XCObjShapePrinter>(meshSubdivisions);
        else
            throw NoSuchShapePrinterException("XenoCollideTraits: unknown printer format: " + format);
    }

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const final {
        if (!this->primaryAxis.has_value())
            throw std::runtime_error("XenoCollideTraits::getPrimaryAxis: primary axis not defined");
        return shape.getOrientation() * this->primaryAxis.value();
    }

    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const final {
        if (!this->secondaryAxis.has_value())
            throw std::runtime_error("XenoCollideTraits::getSecondaryAxis: secondary axis not defined");
        return shape.getOrientation() * this->secondaryAxis.value();
    }

    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const final {
        return shape.getOrientation() * this->geometricOrigin;
    }

    [[nodiscard]] double getVolume([[maybe_unused]] const Shape &shape) const final { return this->volume; }

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasWallPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool isConvex() const override { return true; }

    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                      [[maybe_unused]] const std::byte *data1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2,
                                      [[maybe_unused]] const std::byte *data2, std::size_t idx2,
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
                                       [[maybe_unused]] const std::byte *data, std::size_t idx,
                                       const Vector<3> &wallOrigin, const Vector<3> &wallVector) const override
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

    [[nodiscard]] double getRangeRadius([[maybe_unused]] const std::byte *data) const override {
        if (this->rangeRadius.has_value())
            return *this->rangeRadius;

        // TODO: fix nullptr
        std::size_t numCenters = std::max(this->getInteractionCentres(nullptr).size(), std::size_t{1});
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
