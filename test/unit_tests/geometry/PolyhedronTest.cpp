//
// Created by pkua on 22.11.22.
//

#include <catch2/catch.hpp>

#include "geometry/Polyhedron.h"


TEST_CASE("Polyhedron") {
    Polyhedron::VertexList vertices{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    Polyhedron::TriangleList triangles{{0, 1, 2}, {0, 1, 3}, {1, 2, 3}, {2, 0, 3}};
    Polyhedron polyhedron{{0.1, 0.1, 0.1}, vertices, triangles};

    CHECK(polyhedron.center == Vector<3>{0.1, 0.1, 0.1});
    CHECK(polyhedron.vertices == vertices);
    CHECK(polyhedron.triangles == triangles);

    SECTION("toWolfram") {
        auto expected =
R"(GraphicsComplex[{
    {0.0, 0.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
},
Polygon[{
    {1, 2, 3},
    {1, 2, 4},
    {2, 3, 4},
    {3, 1, 4}
}]]
)";
        std::ostringstream out;
        out.precision(1);
        polyhedron.storeWolframGraphicsComplex(out);
        CHECK(out.str() == expected);
    }

    SECTION("toWavefrontObj") {
        auto expected =
R"(v 0 0 0
v 1 0 0
v 0 1 0
v 0 0 1

f 11 12 13
f 11 12 14
f 12 13 14
f 13 11 14
)";
        std::ostringstream out;
        out.precision(1);
        // Vertex offset by 10
        polyhedron.storeWavefrontObj(out, 10);
        CHECK(out.str() == expected);
    }

    SECTION("volume") {
        CHECK(polyhedron.getVolume() == Approx(1./6));
    }

    SECTION("transformation") {
        auto transformed = polyhedron.transformed({0, 0, 1}, Matrix<3, 3>{-1,  0, 0,
                                                                           0, -1, 0,
                                                                           0,  0, 1});
        CHECK(transformed.center == Vector<3>{0.1, 0.1, 1.1});
        CHECK(transformed.vertices == Polyhedron::VertexList{{0, 0, 1}, {-1, 0, 1}, {0, -1, 1}, {0, 0, 2}});
        CHECK(transformed.triangles == triangles);
    }
}