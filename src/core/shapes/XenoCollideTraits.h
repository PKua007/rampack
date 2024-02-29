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
#include "PolydisperseXCObjShapePrinter.h"
#include "PolydisperseXCWolframShapePrinter.h"
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
class XenoCollideTraits : public ShapeTraits, public Interaction {
private:
    template<typename Printer>
    std::shared_ptr<Printer> createPrinter(std::size_t meshSubdivisions) const {
        using GeometryComplex = PolydisperseXCShapePrinter::GeometryComplex;
        using GeometryComplexProvider = PolydisperseXCShapePrinter::GeometryComplexProvider;

        GeometryComplexProvider provider = [this](const ShapeData &data) {
            auto centers = this->getInteractionCentres(data.raw());
            if (centers.empty())
                centers.push_back({0, 0, 0});

            const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);

            GeometryComplex geometryComplex;
            geometryComplex.reserve(centers.size());

            using Geometry = decltype(thisConcreteTraits.getCollideGeometry(nullptr, 0));
            for (std::size_t i{}; i < centers.size(); i++) {
                const auto &center = centers[i];
                const auto &geometry = thisConcreteTraits.getCollideGeometry(data.raw(), i);
                auto polymorphicGeometry = std::make_shared<PolymorphicXCAdapter<Geometry>>(geometry);
                geometryComplex.emplace_back(center, std::move(polymorphicGeometry));
            }

            return geometryComplex;
        };

        return std::make_shared<Printer>(std::move(provider), meshSubdivisions);
    }

public:
    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    XenoCollideTraits() = default;
    XenoCollideTraits(const XenoCollideTraits &) = delete;
    XenoCollideTraits &operator=(const XenoCollideTraits &) = delete;

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }

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
            return this->template createPrinter<PolydisperseXCWolframShapePrinter>(meshSubdivisions);
        else if (format == "obj")
            return this->template createPrinter<PolydisperseXCObjShapePrinter>(meshSubdivisions);
        else
            throw NoSuchShapePrinterException("XenoCollideTraits: unknown printer format: " + format);
    }

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasWallPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    // TODO: fix isConvex
    [[nodiscard]] bool isConvex() const override { return true; }

    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                      const std::byte *data1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2,
                                      const std::byte *data2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override
    {
        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        const auto &collideGeometry1 = thisConcreteTraits.getCollideGeometry(data1, idx1);
        const auto &collideGeometry2 = thisConcreteTraits.getCollideGeometry(data2, idx2);
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

    [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, const std::byte *data,
                                       std::size_t idx, const Vector<3> &wallOrigin,
                                       const Vector<3> &wallVector) const override
    {
        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        const auto &collideGeometry = thisConcreteTraits.getCollideGeometry(data, idx);

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

    [[nodiscard]] double getRangeRadius(const std::byte *data) const override {
        std::size_t numCenters = std::max(this->getInteractionCentres(data).size(), std::size_t{1});
        const auto &thisConcreteTraits = static_cast<const ConcreteCollideTraits &>(*this);
        double maxRadius{};
        for (std::size_t i{}; i < numCenters; i++) {
            double newRadius = thisConcreteTraits.getCollideGeometry(data, i).getCircumsphereRadius();
            if (newRadius > maxRadius)
                maxRadius = newRadius;
        }
        return 2*maxRadius;
    }
};


#endif //RAMPACK_XENOCOLLIDETRAITS_H
