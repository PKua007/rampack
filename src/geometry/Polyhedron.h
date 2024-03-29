//
// Created by pkua on 17.11.22.
//

#ifndef RAMPACK_POLYHEDRON_H
#define RAMPACK_POLYHEDRON_H

#include <array>
#include <vector>
#include <string>

#include "Vector.h"


/**
 * @brief A helper structure encapsulating a polyhedron mesh.
 */
class Polyhedron {
public:
    using Triangle = std::array<std::size_t, 3>;
    using VertexList = std::vector<Vector<3>>;
    using TriangleList = std::vector<Triangle>;

    /** @brief The center of the polyhedron (as given by @a CollideGeometry::getCenter()). */
    const Vector<3> center;
    /** @brief A list of vertices of the polyhedron. */
    const VertexList vertices;
    /** @brief A list of triangles building the polyhedron. Triangles are triples of inices of vertices. */
    const TriangleList triangles;

    /**
     * @brief Stores Wolfram Mathematica GraphicsComplex representing the polyhedron.
     */
    void storeWolframGraphicsComplex(std::ostream &out) const;
    void storeWavefrontObj(std::ostream &out, std::size_t vertexOffset = 0) const;

    /**
     * @brief Returns the volume of the polyhedron.
     */
    [[nodiscard]] double getVolume() const;

    [[nodiscard]] Polyhedron transformed(const Vector<3> &pos, const Matrix<3, 3> &transform) const;
};


#endif //RAMPACK_POLYHEDRON_H
