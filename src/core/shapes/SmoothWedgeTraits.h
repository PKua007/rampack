//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_SMOOTHWEDGETRAITS_H
#define RAMPACK_SMOOTHWEDGETRAITS_H

#include "XenoCollideTraits.h"
#include "DynamicShapeCache.h"


/**
 * @brief A single species of smooth wedge (a convex hull of two spheres) for SmoothWedgeTraits.
 */
class SmoothWedgeShape /* : public DynamicShapeCache::ConcreteSpecies */  {
public:
    /**
     * @brief Class representing the geometry of the wedge (conforming to XenoCollide @a XCGeometry template parameter).
     */
    class CollideGeometry {
    private:
        double R{};
        double r{};
        double l{};
        double Rminusr{};
        double Rpos{};
        double rpos{};
        double circumsphereRadius{};
        double insphereRadius{};

    public:
        /**
         * @brief Creates the wedge along the z axis, with a sphere of radius @a R at the bottom and a sphere of radius
         * @a r at the top, distance by l.
         * @details They are placed along z axis in such a way, that the point {0, 0, 0} is the center of the best
         * (smallest) circumscribed sphere. Namely, bottom sphere is placed in {0, 0, (@a R - @a r - @a l)/2} and the
         * top one in {0, 0, (@a R - @a r + @a l)/2}.
         * @throws PreconditionException if @a R or @a r are non-positive or if l < abs(R - r)
         */
        CollideGeometry(double R, double r, double l);

        [[nodiscard]] Vector<3> getCenter() const { return {}; }

        [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const {
            Vector<3> nNorm = n.normalized();
            if (this->Rminusr > nNorm[2]*this->l)
                return this->R * nNorm + Vector<3>{0, 0, this->Rpos};
            else
                return this->r * nNorm + Vector<3>{0, 0, this->rpos};
        }

        [[nodiscard]] double getCircumsphereRadius() const { return this->circumsphereRadius; }
        [[nodiscard]] double getInsphereRadius() const { return this->insphereRadius; }
    };

private:
    static double computeVolume(double R, double r, double l);

    [[nodiscard]] std::vector<double> calculateRelativeSpherePositions() const;

    std::vector<CollideGeometry> shapeParts;
    std::vector<Vector<3>> interactionCentres;
    double bottomR{};
    double topR{};
    double l{};
    std::size_t subdivisions{};
    double volume{};
    Vector<3> begNamedPoint{};
    Vector<3> endNamedPoint{};

public:
    /**
     * @brief Creates the wedge species along the z axis. See CollideGeometry::CollideGeometry for the details of
     * how the spheres are placed.
     * @param bottomR corresponds to parameter @a R of CollideGeometry::CollideGeometry
     * @param topR corresponds to parameter @a r of CollideGeometry::CollideGeometry
     * @param l corresponds to parameter @a l of CollideGeometry::CollideGeometry
     * @param subdivisions number of partitions into smaller parts (with multiple interaction centers) for neighbor grid
     * performance reason. Value 0 is treated the same as 1
     * @throws PreconditionException if @a bottomR or @a topR are non-positive or if l < abs(topR - bottomR)
     */
    SmoothWedgeShape(double bottomR, double topR, double l, std::size_t subdivisions = 0) /* override */;

    /**
     * @brief Returns @a true, if the wedge's parameters are as given (see DynamicShapeCache @a ConcreteSpecies template
     * parameter).
     */
    [[nodiscard]] bool equal(double bottomR_, double topR_, double l_, std::size_t subdivisions_) const /* override */;

    [[nodiscard]] double getVolume() const /* override */ { return this->volume; }
    [[nodiscard]] static Vector<3> getPrimaryAxis() /* override */ { return {0, 0, 1}; }
    [[nodiscard]] static Vector<3> getSecondaryAxis() /* override */ {
        throw std::runtime_error("SmoothWedgeShape::getPrimaryAxis: primary axis not defined");
    }
    [[nodiscard]] static Vector<3> getGeometricOrigin() /* override */ { return {0, 0, 0}; }

    [[nodiscard]] double getBottomR() const { return this->bottomR; }
    [[nodiscard]] double getTopR() const { return this->topR; }
    [[nodiscard]] double getL() const { return this->l; }
    [[nodiscard]] std::size_t getSubdivisions() const { return this->subdivisions; }
    [[nodiscard]] const std::vector<CollideGeometry> &getSubdividedGeometries() const { return this->shapeParts; }
    [[nodiscard]] const std::vector<Vector<3>> &getInteractionCentres() const { return this->interactionCentres; }
    [[nodiscard]] const Vector<3> &getBegNamedPoint() const { return this->begNamedPoint; }
    [[nodiscard]] const Vector<3> &getEndNamedPoint() const { return this->endNamedPoint; }
};


/**
 * @brief Class representing a smooth wedge - a convex hull of two spheres with different radii.
 */
class SmoothWedgeTraits
        : public XenoCollideTraits<SmoothWedgeTraits>, public DynamicShapeCache<SmoothWedgeShape>
{
private:
    template<typename Printer>
    std::shared_ptr<Printer> createPrinter(std::size_t meshSubdivisions) const {
        PolydisperseXCShapePrinter::GeometryProvider provider = [this](const ShapeData &data) {
            const auto &shape = this->speciesFor(data);
            CollideGeometry geometry(shape.getBottomR(), shape.getTopR(), shape.getL());
            return std::make_shared<PolymorphicXCAdapter<CollideGeometry>>(geometry);
        };
        return std::make_shared<Printer>(std::move(provider), meshSubdivisions);
    }

public:
    using CollideGeometry = SmoothWedgeShape::CollideGeometry;

    /**
     * @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::buildPolyhedron
     * @a subdivisions parameter)
     */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 4;

    /**
     * @brief Creates a wedge, for which one can optionally define default values of parameters.
     * @details The default-able parameters correspond to the ones of SmoothWedgeShape::SmoothWedgeShape. If
     * @a subdivision is at least two, the wedge is divided into that many parts (with equal circumscribed spheres'
     * radii) to lower the number of neighbours in the neighbour grid.
     */
    explicit SmoothWedgeTraits(std::optional<double> defaultBottomR = std::nullopt,
                               std::optional<double> defaultTopR = std::nullopt,
                               std::optional<double> defaultL = std::nullopt, std::size_t defaultSubdivisions = 0);

    SmoothWedgeTraits(const SmoothWedgeTraits &) = delete;
    SmoothWedgeTraits &operator=(const SmoothWedgeTraits &) = delete;

    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

    [[nodiscard]] bool isConvex() const override { return true; }



    /**
     * @brief Serializes the wedge into a map with shape parameters named `bottom_r`, `top_r`, `l`, and `subdivisions`.
     */
    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const override;

    /**
     * @brief Deserializes the wedge from a map with shape parameters named `bottom_r`, `top_r`, `l`, and
     * `subdivisions`.
     * @throws ShapeDataSerializationException if the keys are incorrect or the values are not numbers
     * @throws ShapeDataFormatException if the resulting values would throw an exception in
     * SmoothWedgeShape::SmoothWedgeShape
     */
    [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const override;

    /**
     * @brief Returns CollideGeometry object for the interaction center with index @a idx (see XenoCollideTraits).
     */
    [[nodiscard]] const CollideGeometry &getCollideGeometry(const std::byte *data, std::size_t idx = 0) const {
        return this->speciesFor(data).getSubdividedGeometries()[idx];
    }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres(const std::byte *data) const override {
        return this->speciesFor(data).getInteractionCentres();
    }

    /**
     * @brief Returns ShapePrinter for a given @a format.
     * @details The following formats are supported:
     * <ol>
     *     <li> `wolfram` - Wolfram Mathematica shape
     *     <li> `obj` - Wavefront OBJ triangle mesh (it accepts @a mesh_divisions parameter, default: 4)
     * </ol>
     */
    [[nodiscard]] std::shared_ptr<const ShapePrinter>
    getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const override;

    /**
     * @brief Returns the ShapeData for given parameters (registering it in the DynamicShapeCache). The parameters
     * (and exception) correspond to the ones of SmoothWedgeShape::SmoothWedgeShape.
     */
    ShapeData shapeDataForSpecies(double bottomR, double topR, double l, std::size_t subdivisions = 0) const;
};


#endif //RAMPACK_SMOOTHWEDGETRAITS_H
