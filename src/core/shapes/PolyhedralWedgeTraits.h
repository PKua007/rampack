//
// Created by ciesla on 12/04/23.
//

#ifndef RAMPACK_POLYHEDRALWEDGETRAITS_H
#define RAMPACK_POLYHEDRALWEDGETRAITS_H

#include "XenoCollideTraits.h"
#include "DynamicShapeCache.h"


/**
 * @brief A single species of polyhedral wedge (a convex hull of two axis-aligned rectangles) for PolyhedralWedgeShape.
 */
class PolyhedralWedgeShape /* : public DynamicShapeCache::ConcreteSpecies */ {
public:
    /**
     * @brief Class representing the geometry of the polyhedral wedge (conforming to XenoCollide @a XCGeometry template
     * parameter).
     */
    class CollideGeometry {
    private:
        Vector<3> vertexUp;
        Vector<3> vertexDown;
        double circumsphereRadius{};
        double insphereRadius{};

    public:
        /**
         * @brief Creates the polyhedral wedge along the z axis, with a rectangle (@a axTop, @a ayTop) at the top and
         * (@a axBottom, @a ayBottom) at the bottom. The distance between rectangles is @a length.
         * @details The bottom rectangle has the center at {0, 0, @a -length / 2)} and the top rectangle has the center
         * at {0, 0, @a length / 2)}.
         */
        CollideGeometry(double bottomAx, double bottomAy, double topAx, double topAy, double l);

        [[nodiscard]] Vector<3> getCenter() const { return {}; }

        [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const {
            auto rUp = this->vertexUp;
            auto rDown = this->vertexDown;

            if (n[0] < 0) {
                rUp[0] = -rUp[0];
                rDown[0] = -rDown[0];
            }
            if (n[1] < 0) {
                rUp[1] = -rUp[1];
                rDown[1] = -rDown[1];
            }

            // Convex hull (max of support function)
            if ((rUp - rDown) * n > 0)
                return rUp;
            else
                return rDown;
        }

        [[nodiscard]] double getCircumsphereRadius() const { return this->circumsphereRadius; }
        [[nodiscard]] double getInsphereRadius() const { return this->insphereRadius; }
    };

private:
    static double computeVolume(double bottomAx, double bottomAy, double topAx, double topAy, double l);

    std::vector<CollideGeometry> shapeParts;
    std::vector<Vector<3>> interactionCentres;
    double bottomAx{};
    double bottomAy{};
    double topAx{};
    double topAy{};
    double l{};
    std::size_t subdivisions{};
    double volume{};
    Vector<3> begNamedPoint{};
    Vector<3> endNamedPoint{};

public:
    /**
     * @brief Creates the polyhedral wedge species along the z axis. See CollideGeometry::CollideGeometry for the
     * details of how the rectangles are placed.
     * @details The rectangles can be reduces to lines, but the volume of the resulting shape must be positive.
     * @param bottomAx corresponds to parameter @a bottomAx of CollideGeometry::CollideGeometry
     * @param bottomAy corresponds to parameter @a bottomAy of CollideGeometry::CollideGeometry
     * @param topAx corresponds to parameter @a topAx of CollideGeometry::CollideGeometry
     * @param topAy corresponds to parameter @a topAy of CollideGeometry::CollideGeometry
     * @param l corresponds to parameter @a l of CollideGeometry::CollideGeometry
     * @param subdivisions number of partitions into smaller parts (with multiple interaction centers) for neighbor grid
     * performance reason. Value 0 is treated the same as 1
     * @throws PreconditionException is the parameters would yield a zero volume
     */
    PolyhedralWedgeShape(double bottomAx, double bottomAy, double topAx, double topAy, double l,
                         std::size_t subdivisions = 0) /* override */;

    /**
     * @brief Returns @a true, if the wedge's parameters are as given (see DynamicShapeCache @a ConcreteSpecies template
     * parameter).
     */
    [[nodiscard]] bool equal(double bottomAx_, double bottomAy_, double topAx_, double topAy_, double l_,
                             std::size_t subdivisions_) const /* override */;

    [[nodiscard]] double getBottomAx() const { return this->bottomAx; }
    [[nodiscard]] double getBottomAy() const { return this->bottomAy; }
    [[nodiscard]] double getTopAx() const { return this->topAx; }
    [[nodiscard]] double getTopAy() const { return this->topAy; }
    [[nodiscard]] double getL() const { return this->l; }
    [[nodiscard]] std::size_t getSubdivisions() const { return this->subdivisions; }
    [[nodiscard]] const std::vector<CollideGeometry> &getSubdividedGeometries() const { return this->shapeParts; }
    [[nodiscard]] const std::vector<Vector<3>> &getInteractionCentres() const { return this->interactionCentres; }
    [[nodiscard]] double getVolume() const { return this->volume; }
    [[nodiscard]] const Vector<3> &getBegNamedPoint() const { return this->begNamedPoint; }
    [[nodiscard]] const Vector<3> &getEndNamedPoint() const { return this->endNamedPoint; }
};

/**
 * @brief Class representing a wedge built by taking a convex hull of two axis-oriented rectangles.
 */
class PolyhedralWedgeTraits
        : public XenoCollideTraits<PolyhedralWedgeTraits>,
          public DynamicShapeCache<PolyhedralWedgeShape>,
          public ShapeGeometry
{
private:
    template<typename Printer>
    std::shared_ptr<Printer> createPrinter(std::size_t meshSubdivisions) const {
        PolydisperseXCShapePrinter::GeometryProvider provider = [this](const ShapeData &data) {
            const auto &shape = this->speciesFor(data);
            CollideGeometry geometry(shape.getBottomAx(), shape.getBottomAy(), shape.getTopAx(), shape.getTopAy(),
                                     shape.getL());
            return std::make_shared<PolymorphicXCAdapter<CollideGeometry>>(geometry);
        };
        return std::make_shared<Printer>(std::move(provider), meshSubdivisions);
    }

public:
    using CollideGeometry = PolyhedralWedgeShape::CollideGeometry;

    /**
     * @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::buildPolyhedron
     * @a subdivisions parameter)
     */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 4;

    /**
     * @brief Creates a wedge, for which one can optionally define default values of parameters.
     * @details The default-able parameters correspond to the ones of PolyhedralWedgeShape::PolyhedralWedgeShape. If
     * @a subdivision is at least two, the wedge is divided into that many parts along the length to lower the number of
     * neighbours in the neighbour grid.
     */
    explicit PolyhedralWedgeTraits(std::optional<double> defaultBottomAx = std::nullopt,
                                   std::optional<double> defaultBottomAy = std::nullopt,
                                   std::optional<double> defaultTopAx = std::nullopt,
                                   std::optional<double> defaultTopAy = std::nullopt,
                                   std::optional<double> defaultL = std::nullopt, std::size_t defaultSubdivisions = 0);

    PolyhedralWedgeTraits(const PolyhedralWedgeTraits &) = delete;
    PolyhedralWedgeTraits &operator=(const PolyhedralWedgeTraits &) = delete;

    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

    [[nodiscard]] bool isConvex() const override { return true; }

    [[nodiscard]] double getVolume(const Shape &shape) const override { return this->speciesFor(shape).getVolume(); }
    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override {
        return shape.getOrientation().column(2);
    }
    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const override {
        return shape.getOrientation().column(0);
    }
    [[nodiscard]] Vector<3> getGeometricOrigin([[maybe_unused]] const Shape &shape) const override { return {0, 0, 0}; }

    /**
     * @brief Serializes the wedge into a map with shape parameters named `bottom_ax`, `bottom_ay`, `top_ax`, `top_ay`,
     * `l`, and `subdivisions`.
     */
    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const override;

    /**
     * @brief Deserializes the wedge from a map with shape parameters named `bottom_ax`, `bottom_ay`, `top_ax`,
     * `top_ay`, `l`, and `subdivisions`.
     * @throws ShapeDataSerializationException if the keys are incorrect or the values are not numbers
     * @throws ShapeDataFormatException if the resulting values would throw an exception in
     * PolyhedralWedgeShape::PolyhedralWedgeShape
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
     * (and exception) correspond to the ones of PolyhedralWedgeShape::PolyhedralWedgeShape.
     */
    ShapeData shapeDataForSpecies(double bottomAx, double bottomAy, double topAx, double topAy, double l,
                                  std::size_t subdivisions = 0) const;
};


#endif //RAMPACK_POLYHEDRALWEDGETRAITS_H
