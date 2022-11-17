//
// Created by pkua on 04.10.22.
//

#ifndef RAMPACK_XCPRINTER_H
#define RAMPACK_XCPRINTER_H

#include <utility>
#include <array>
#include <vector>

#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "geometry/Polyhedron.h"


/**
 * @brief A class responsible for creating meshes for a given CollideGeometry.
 */
class XCPrinter {
private:
    static Polyhedron::VertexList generateSpherePoints(std::size_t subdivisions);
    static Polyhedron buildPolyhedron0(const AbstractXCGeometry &geometry, std::size_t subdivisions);

public:
    /**
     * @brief For a given @a geometry it creates a mesh representing it.
     * @details The mesh is created by sampling points on a sphere (by recursively dividing octahedron triangle into 4
     * smaller ones @a subdivisions times and projecting them on a sphere in each iteration). Then those points are fed
     * to @a CollideGeometry::getSupportPoint which maps them to a set of points on the shape's boundary. The final
     * polyhedron is created by triangulating those points.
     * @tparam CollideGeometry a class conforming to requirements of the template parameter of XenoCollide
     */
    template<typename CollideGeometry>
    static Polyhedron buildPolyhedron(const CollideGeometry &geometry, std::size_t subdivisions) {
        if constexpr (std::is_convertible_v<const CollideGeometry&, const AbstractXCGeometry&>)
            return buildPolyhedron0(geometry, subdivisions);
        else
            return buildPolyhedron0(PolymorphicXCAdapter<CollideGeometry>(geometry), subdivisions);
    }
};


#endif //RAMPACK_XCPRINTER_H
