//
// Created by pkua on 04.10.22.
//

#ifndef RAMPACK_XCPRINTER_H
#define RAMPACK_XCPRINTER_H

#include <utility>
#include <array>
#include <vector>

#include "geometry/xenocollide/AbstractXCGeometry.h"


class XCPrinter {
public:
    using Triangle = std::array<std::size_t, 3>;
    using VertexList = std::vector<Vector<3>>;
    using TriangleList = std::vector<Triangle>;

    struct Polyhedron {
        const Vector<3> center;
        const VertexList vertices;
        const TriangleList triangles;

        [[nodiscard]] std::string toWolfram() const;
        [[nodiscard]] double getVolume() const;
    };

private:
    static VertexList generateSpherePoints(std::size_t subdivisions);

public:
    static Polyhedron buildPolyhedron(const AbstractXCGeometry &geometry, std::size_t subdivisions);

    template<typename CollideGeometry>
    static Polyhedron buildPolyhedron(CollideGeometry geometry, std::size_t subdivisions) {
        return buildPolyhedron(PolymorphicCollideAdapter<CollideGeometry>(std::move(geometry)));
    }
};


#endif //RAMPACK_XCPRINTER_H
