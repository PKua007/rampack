//
// Created by ciesla on 12/04/23.
//

#ifndef RAMPACK_POLYHEDRALWEDGE_H
#define RAMPACK_POLYHEDRALWEDGE_H

#include "XenoCollideTraits.h"


/**
 * @brief Class representing a distorted tetrahedron - a convex hull based on two perpendicular segments of length
 * R and r, placed in XY planes and the distance between them along the Z axis equals l.
 */
class PolyhedralWedge : public XenoCollideTraits<PolyhedralWedge> {
public:
    /**
     * @brief Class representing the geometry of the distorted tetrahedron (see template parameter of XenoCollide)
     */
    class CollideGeometry {
    private:
        Vector<3> vertexUp;
        Vector<3> vertexDown;
        double circumsphereRadius{};
        double insphereRadius{};

        friend PolyhedralWedge;

    public:
        /**
         * @brief Creates the distorted tetrahedron along the z axis, with a rectangle (@a 2rxUp, @a 2ryUp) at the top and
         * (@a 2rxDown, @a 2ryDown) at the bottom. The distance between rectangles is length.
         * @details The bottom rectangle is placed along X axis with the center at {0, 0, - @a length)/2} and
         * the top rectangle is placed along Y axis with the center at {0, 0, @a length)/2}.
         */
        CollideGeometry(double rxTop, double ryTop, double rxBottom, double ryBottom, double length);

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
    double rxTop{}, ryTop{};
    double rxBottom, ryBottom{};
    double length{};
    std::vector<CollideGeometry> shapeModels;
    std::vector<Vector<3>> interactionCentres;

    static double getVolume(double rxTop, double ryTop, double rxBottom, double ryBottom, double length);

    template<typename Printer>
    std::shared_ptr<Printer> createPrinter(std::size_t meshSubdivisions) const {
        CollideGeometry geometry(this->rxTop, this->ryTop, this->rxBottom, this->ryBottom, this->length);
        PolymorphicXCAdapter<CollideGeometry> geometryAdapter(geometry);
        return std::make_shared<Printer>(geometryAdapter, meshSubdivisions);
    }

public:
    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 4;

    /**
     * @brief Creates a wedge with parameters @a R, @a r and @a length (see CollideGeometry::CollideGeometry).
     * @details If @a subdivision is at least two, the wedge is divided into that many parts (with equal circumscribed
     * spheres' radii) to lower the number of neighbours in the neighbour grid.
     */
    PolyhedralWedge(double rxTop, double ryTop, double rxBottom, double ryBottom, double length, std::size_t subdivisions = 0);

    /**
     * @brief Returns CollideGeometry object for the interaction center with index @a idx (see XenoCollideTraits).
     */
    [[nodiscard]] const CollideGeometry &getCollideGeometry([[maybe_unused]] std::size_t idx = 0) const {
        return this->shapeModels[idx];
    }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override { return this->interactionCentres; }

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
};


#endif //RAMPACK_POLYHEDRALWEDGE_H
