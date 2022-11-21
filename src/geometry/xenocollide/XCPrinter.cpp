//
// Created by pkua on 04.10.22.
//

#include <algorithm>
#include <iterator>
#include <sstream>
#include <QuickHull.hpp>

#include "XCPrinter.h"
#include "utils/Assertions.h"


Polyhedron XCPrinter::buildPolyhedron0(const AbstractXCGeometry &geometry, std::size_t subdivisions) {
    auto spherePoints = XCPrinter::generateSpherePoints(subdivisions);

    using QHVector = quickhull::Vector3<double>;
    std::vector<QHVector> hullPoints;
    hullPoints.reserve(spherePoints.size());
    auto supportMapping = [&geometry](const Vector<3> &v) {
        auto support = geometry.getSupportPoint(v);
        return QHVector{support[0], support[1], support[2]};
    };
    std::transform(spherePoints.begin(), spherePoints.end(), std::back_inserter(hullPoints), supportMapping);

    quickhull::QuickHull<double> qh;
    auto hull = qh.getConvexHull(hullPoints, true, false);
    const auto &indexBuffer = hull.getIndexBuffer();
    const auto &vertexBuffer = hull.getVertexBuffer();
    Assert(indexBuffer.size() % 3 == 0);

    Polyhedron::VertexList vertices;
    vertices.reserve(vertexBuffer.size());
    auto vectorConverter = [](const QHVector &vec) {
        return Vector<3>{vec.x, vec.y, vec.z};
    };
    std::transform(vertexBuffer.begin(), vertexBuffer.end(), std::back_inserter(vertices), vectorConverter);

    Polyhedron::TriangleList triangles;
    triangles.reserve(indexBuffer.size() / 3);
    Vector<3> center = geometry.getCenter();
    for (std::size_t i{}; i < indexBuffer.size(); i += 3) {
        Polyhedron::Triangle triangle{indexBuffer[i], indexBuffer[i + 1], indexBuffer[i + 2]};

        // Make sure that triangle's normal points outwards
        Vector<3> s1 = vertices[triangle[1]] - vertices[triangle[0]];
        Vector<3> s2 = vertices[triangle[2]] - vertices[triangle[1]];
        Vector<3> r = vertices[triangle[0]] - center;
        if ((s1 ^ s2) * r < 0)
            std::swap(triangle[0], triangle[1]);

        triangles.push_back(triangle);
    }

    return Polyhedron{center, std::move(vertices), std::move(triangles)};
}

Polyhedron::VertexList XCPrinter::generateSpherePoints(std::size_t subdivisions) {
    Polyhedron::VertexList verts = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    Polyhedron::TriangleList tris = {{0, 3, 5}, {2, 0, 5}, {1, 2, 5}, {3, 1, 5},
                         {3, 0, 4}, {0, 2, 4}, {2, 1, 4}, {1, 3, 4}};

    for (std::size_t i{}; i < subdivisions; i++) {
        Polyhedron::TriangleList newTris;
        for (const auto &tri : tris) {
            std::size_t backIdx = verts.size();
            verts.push_back((verts[tri[0]] + verts[tri[1]]).normalized());
            verts.push_back((verts[tri[1]] + verts[tri[2]]).normalized());
            verts.push_back((verts[tri[2]] + verts[tri[0]]).normalized());

            newTris.push_back({tri[0], backIdx, backIdx + 2});
            newTris.push_back({backIdx, tri[1], backIdx + 1});
            newTris.push_back({backIdx + 2, backIdx + 1, tri[2]});
            newTris.push_back({backIdx, backIdx + 1, backIdx + 2});
        }
        std::swap(tris, newTris);
    }

    return verts;
}
