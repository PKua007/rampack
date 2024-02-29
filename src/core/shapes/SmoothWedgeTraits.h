//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_SMOOTHWEDGETRAITS_H
#define RAMPACK_SMOOTHWEDGETRAITS_H

#include "XenoCollideTraits.h"


class SmoothWedgeShape {
public:
    /**
     * @brief Class representing the geometry of the wedge (see template parameter of XenoCollide)
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

    double bottomR{};
    double topR{};
    double l{};
    std::size_t subdivisions{};
    double volume{};
    Vector<3> begNamedPoint{};
    Vector<3> endNamedPoint{};
    std::vector<CollideGeometry> shapeParts;
    std::vector<Vector<3>> interactionCentres;

public:
    SmoothWedgeShape(double bottomR, double topR, double l, std::size_t subdivisions = 0);

    [[nodiscard]] double getBottomR() const { return this->bottomR; }
    [[nodiscard]] double getTopR() const { return this->topR; }
    [[nodiscard]] double getL() const { return this->l; }
    [[nodiscard]] std::size_t getSubdivisions() const { return this->subdivisions; }
    [[nodiscard]] const std::vector<CollideGeometry> &getShapeParts() const { return this->shapeParts; }
    [[nodiscard]] const std::vector<Vector<3>> &getInteractionCentres() const { return this->interactionCentres; }
    [[nodiscard]] double getVolume() const { return this->volume; }
    [[nodiscard]] const Vector<3> &getBegNamedPoint() const { return this->begNamedPoint; }
    [[nodiscard]] const Vector<3> &getEndNamedPoint() const { return this->endNamedPoint; }
    [[nodiscard]] bool equal(double bottomR_, double topR_, double l_, std::size_t subdivisions_) const;
};


/**
 * @brief Class representing a smooth wedge - a convex hull of two spheres with different radii.
 */
class SmoothWedgeTraits : public XenoCollideTraits<SmoothWedgeTraits>, public ShapeGeometry, public ShapeDataManager {
private:
    struct Data {
        std::size_t shapeIdx{};

        friend bool operator==(Data lhs, Data rhs) {
            return lhs.shapeIdx == rhs.shapeIdx;
        }
    };

    /*template<typename Printer>
    std::shared_ptr<Printer> createPrinter(std::size_t meshSubdivisions) const {
        CollideGeometry geometry(this->R, this->r, this->l);
        PolymorphicXCAdapter<CollideGeometry> geometryAdapter(geometry);
        return std::make_shared<Printer>(geometryAdapter, meshSubdivisions);
    }*/

    [[nodiscard]] const SmoothWedgeShape &shapeFor(const std::byte *data) const {
        std::size_t shapeIdx = ShapeData::as<Data>(data).shapeIdx;
        return this->shapes[shapeIdx];
    }

    [[nodiscard]] const SmoothWedgeShape &shapeFor(const ShapeData &data) const;

    [[nodiscard]] const SmoothWedgeShape &shapeFor(const Shape &shape) const {
        return this->shapeFor(shape.getData());
    }

    std::optional<double> defaultBottomR{};
    std::optional<double> defaultTopR{};
    std::optional<double> defaultL{};
    std::size_t defaultSubdivisions = 1;
    mutable std::vector<SmoothWedgeShape> shapes;

public:
    using CollideGeometry = SmoothWedgeShape::CollideGeometry;

    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 4;

    /**
     * @brief Creates a wedge with parameters @a R, @a r and @a l (see CollideGeometry::CollideGeometry).
     * @details If @a subdivision is at least two, the wedge is divided into that many parts (with equal circumscribed
     * spheres' radii) to lower the number of neighbours in the neighbour grid.
     */
    explicit SmoothWedgeTraits(std::optional<double> defaultBottomR = std::nullopt,
                               std::optional<double> defaultTopR = std::nullopt,
                               std::optional<double> defaultL = std::nullopt, std::size_t defaultSubdivisions = 0);

    SmoothWedgeTraits(const SmoothWedgeTraits &) = delete;
    SmoothWedgeTraits &operator=(const SmoothWedgeTraits &) = delete;

    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

    [[nodiscard]] double getVolume(const Shape &shape) const override { return this->shapeFor(shape).getVolume(); }
    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override { return shape.getOrientation().column(2); }
    [[nodiscard]] Vector<3> getGeometricOrigin([[maybe_unused]] const Shape &shape) const override { return {0, 0, 0}; }

    [[nodiscard]] std::size_t getShapeDataSize() const override { return sizeof(Data); }
    void validateShapeData(const ShapeData &data) const override;
    [[nodiscard]] ShapeData::Comparator getComparator() const override { return ShapeData::Comparator::forType<Data>(); }
    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const override;
    [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const override;

    /**
     * @brief Returns CollideGeometry object for the interaction center with index @a idx (see XenoCollideTraits).
     */
    [[nodiscard]] const CollideGeometry &getCollideGeometry(const std::byte *data, std::size_t idx = 0) const {
        return this->shapeFor(data).getShapeParts()[idx];
    }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres(const std::byte *data) const override {
        return this->shapeFor(data).getInteractionCentres();
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

    ShapeData shapeDataFor(double bottomR, double topR, double l, std::size_t subdivisions = 0) const;
};


#endif //RAMPACK_SMOOTHWEDGETRAITS_H
