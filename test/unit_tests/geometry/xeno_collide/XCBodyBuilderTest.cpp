//
// Created by pkua on 08.10.22.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "geometry/xenocollide/XCBodyBuilder.h"

// Uncomment if you want to visualize shapes in each test case with their inspheres and circumspheres
//#define VISUALIZE

#ifdef VISUALIZE
    #include <iostream>
    #include "geometry/xenocollide/XCPrinter.h"
#endif

namespace {
    void visualize([[maybe_unused]] const AbstractXCGeometry &geom) {
        #ifdef VISUALIZE
            auto polyhedron = XCPrinter::buildPolyhedron(geom, 4);
            double r = geom.getInsphereRadius();
            double R = geom.getCircumsphereRadius();
            std::cout << "Graphics3D[{{Red,Sphere[{0,0,0}," << r << "]}," << std::endl;
            std::cout << "{White,Opacity[0.5]," << polyhedron.toWolfram() << "}," << std::endl;
            std::cout << "{Cyan,Opacity[0.25],Sphere[{0,0,0}," << R << "]}}]" << std::endl;
        #endif
    }
}


TEST_CASE("XCBodyBuilder: cuboid") {
    XCBodyBuilder builder;

    builder.processCommand("cuboid 1 2 3");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK(geom->getInsphereRadius() == Approx(0.5));
    CHECK(geom->getCircumsphereRadius() == Approx(std::sqrt(1.*1 + 2*2 + 3*3)/2));
    CHECK((geom->getSupportPoint({2, 0, 0}) * Vector<3>{1, 0, 0}) == Approx(0.5));
    CHECK((geom->getSupportPoint({-2, 0, 0}) * Vector<3>{1, 0, 0}) == Approx(-0.5));
    CHECK((geom->getSupportPoint({0, 2, 0}) * Vector<3>{0, 1, 0}) == Approx(1));
    CHECK((geom->getSupportPoint({0, -2, 0}) * Vector<3>{0, 1, 0}) == Approx(-1));
    CHECK((geom->getSupportPoint({0, 0, 2}) * Vector<3>{0, 0, 1}) == Approx(1.5));
    CHECK((geom->getSupportPoint({0, 0, -2}) * Vector<3>{0, 0, 1}) == Approx(-1.5));
}

TEST_CASE("XCBodyBuilder: disk") {
    XCBodyBuilder builder;

    builder.processCommand("disk 2");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK(geom->getInsphereRadius() == 0);
    CHECK(geom->getCircumsphereRadius() == 2);
    CHECK_THAT(geom->getSupportPoint({0, 0, 2}), IsApproxEqual({}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({1, 2, 2}), IsApproxEqual(2.*Vector<3>{1, 2, 0}.normalized(), 1e-12));
}

TEST_CASE("XCBodyBuilder: ellipse") {
    XCBodyBuilder builder;

    builder.processCommand("ellipse 1 2");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK(geom->getInsphereRadius() == 0);
    CHECK(geom->getCircumsphereRadius() == 2);
    CHECK_THAT(geom->getSupportPoint({0, 0, 2}), IsApproxEqual({}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({2, 0, 2}), IsApproxEqual({1, 0, 0}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({0, 2, 2}), IsApproxEqual({0, 2, 0}, 1e-12));
}

TEST_CASE("XCBodyBuilder: ellipsoid") {
    XCBodyBuilder builder;

    builder.processCommand("ellipsoid 1 2 3");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK(geom->getInsphereRadius() == 1);
    CHECK(geom->getCircumsphereRadius() == 3);
    CHECK_THAT(geom->getSupportPoint({2, 0, 0}), IsApproxEqual({1, 0, 0}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({0, 2, 0}), IsApproxEqual({0, 2, 0}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({0, 0, 2}), IsApproxEqual({0, 0, 3}, 1e-12));
}

TEST_CASE("XCBodyBuilder: football") {
    XCBodyBuilder builder;

    builder.processCommand("football 4 1");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK(geom->getInsphereRadius() == 1);
    CHECK(geom->getCircumsphereRadius() == 2);

    double l = 2, r = 1;
    double x = 0.5*l*l/r - 1.5*r;
    double R = 0.5*(l*l/r + r);
    // Curved part - X direction
    Vector<3> curvedN{x + r, 0, -l + 0.1};
    Vector<3> curvedP = Vector<3>{-(r + x), 0, 0} + R*curvedN.normalized();
    CHECK_THAT(geom->getSupportPoint(curvedN), IsApproxEqual(curvedP, 1e-12));
    // Curved part - Y direction
    Vector<3> curvedN2{ 0, x + r, -l + 0.1};
    Vector<3> curvedP2 = Vector<3>{0, -(r + x), 0} + R*curvedN2.normalized();
    CHECK_THAT(geom->getSupportPoint(curvedN2), IsApproxEqual(curvedP2, 1e-12));
    // The very tip
    Vector<3> notCurvedN{x + r, 0, -l - 0.1};
    CHECK_THAT(geom->getSupportPoint(notCurvedN), IsApproxEqual({0, 0, -l}, 1e-12));
}

TEST_CASE("XCBodyBuilder: point") {
    XCBodyBuilder builder;

    builder.processCommand("point 1 2 3");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({1, 2, 3}, 1e-12));
    CHECK(geom->getInsphereRadius() == 0);
    CHECK(geom->getCircumsphereRadius() == 0);
    CHECK_THAT(geom->getSupportPoint({1, 4, -7}), IsApproxEqual({1, 2, 3}, 1e-12));
}

TEST_CASE("XCBodyBuilder: rectangle") {
    XCBodyBuilder builder;

    builder.processCommand("rectangle 1 2");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK(geom->getInsphereRadius() == 0);
    CHECK(geom->getCircumsphereRadius() == Approx(std::sqrt(1.*1 + 2*2)/2));
    CHECK(geom->getSupportPoint({0, 0, 2}) * Vector<3>{0, 0, 1} == Approx(0).margin(1e-12));
    CHECK((geom->getSupportPoint({2, 0, 2}) * Vector<3>{1, 0, 0}) == Approx(0.5));
    CHECK((geom->getSupportPoint({-2, 0, 2}) * Vector<3>{1, 0, 0}) == Approx(-0.5));
    CHECK((geom->getSupportPoint({0, 2, 2}) * Vector<3>{0, 1, 0}) == Approx(1));
    CHECK((geom->getSupportPoint({0, -2, 2}) * Vector<3>{0, 1, 0}) == Approx(-1));

}

TEST_CASE("XCBodyBuilder: saucer") {
    XCBodyBuilder builder;

    builder.processCommand("saucer 2 2");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK(geom->getInsphereRadius() == 1);
    CHECK(geom->getCircumsphereRadius() == 2);

    double l = 2, r = 1;
    double x = 0.5*l*l/r - 1.5*r;
    double R = 0.5*(l*l/r + r);
    // Curved part
    Vector<3> curvedN{-l + 0.1, 0, x + r};
    Vector<3> curvedP = Vector<3>{0, 0, -(r + x)} + R*curvedN.normalized();
    CHECK_THAT(geom->getSupportPoint(curvedN), IsApproxEqual(curvedP, 1e-12));
    // The rim - Y direction
    Vector<3> notCurvedN1{-l - 0.1, 0, x + r};
    CHECK_THAT(geom->getSupportPoint(notCurvedN1), IsApproxEqual({-l, 0, 0}, 1e-12));
    // The rim - Z direction
    Vector<3> notCurvedN2{0, -l - 0.1, x + r};
    CHECK_THAT(geom->getSupportPoint(notCurvedN2), IsApproxEqual({0, -l, 0}, 1e-12));

}

TEST_CASE("XCBodyBuilder: segment") {
    XCBodyBuilder builder;

    builder.processCommand("segment 3");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK(geom->getInsphereRadius() == 0);
    CHECK(geom->getCircumsphereRadius() == 1.5);
    CHECK_THAT(geom->getSupportPoint({2, 4, -7}), IsApproxEqual({0, 0, -1.5}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({-2, 4, 7}), IsApproxEqual({0, 0, 1.5}, 1e-12));
}

TEST_CASE("XCBodyBuilder: sphere") {
    XCBodyBuilder builder;

    builder.processCommand("sphere 2");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK(geom->getInsphereRadius() == 2);
    CHECK(geom->getCircumsphereRadius() == 2);
    CHECK_THAT(geom->getSupportPoint({2, 4, -7}), IsApproxEqual(2.*Vector<3>{2, 4, -7}.normalized(), 1e-12));
}

TEST_CASE("XCBodyBuilder: rot + sum") {
    XCBodyBuilder builder;

    builder.processCommand("segment 1");
    builder.processCommand("segment 2");
    builder.processCommand("rot 0 90 0");
    builder.processCommand("sum");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({1, 1, 1}), IsApproxEqual({1, 0, 0.5}, 1e-12));
}

TEST_CASE("XCBodyBuilder: move + rot + diff") {
    XCBodyBuilder builder;

    builder.processCommand("segment 1");
    builder.processCommand("segment 2");
    builder.processCommand("move 0 0 0.5");
    builder.processCommand("rot 0 90 0");
    builder.processCommand("diff");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, -0.5}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({1, 1, 1}), IsApproxEqual({1, 0, 0}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({1, 1, -1}), IsApproxEqual({1, 0, -1}, 1e-12));
}

TEST_CASE("XCBodyBuilder: rot + wrap") {
    XCBodyBuilder builder;

    builder.processCommand("segment 1");
    builder.processCommand("segment 2");
    builder.processCommand("rot 0 90 0");
    builder.processCommand("wrap");
    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({0, 1, 1}), IsApproxEqual({0, 0, 0.5}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({1, 1, 0}), IsApproxEqual({1, 0, 0}, 1e-12));
}

TEST_CASE("XCBodyBuilder: dup + move + wrap") {
    XCBodyBuilder builder;

    // We create a triangular prism
    builder.processCommand("segment 1");
    builder.processCommand("move 0 0.5 0");
    builder.processCommand("segment 1");
    builder.processCommand("move -0.5 -0.5 0");
    builder.processCommand("dup 1");
    builder.processCommand("move 1 0 0");
    builder.processCommand("wrap");
    builder.processCommand("wrap");

    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({0, 1, 0.1}), IsApproxEqual({0, 0.5, 0.5}, 1e-12));
    CHECK_THAT(geom->getSupportPoint({1, -1, 1}), IsApproxEqual({0.5, -0.5, 0.5}, 1e-12));
}

TEST_CASE("XCBodyBuilder: swap + pop") {
    XCBodyBuilder builder;

    // We want to see only a sphere at the end
    builder.processCommand("cuboid 1 2 3");
    builder.processCommand("sphere 2");
    builder.processCommand("swap");
    builder.processCommand("pop");

    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getSupportPoint({2, 4, -7}), IsApproxEqual(2.*Vector<3>{2, 4, -7}.normalized(), 1e-12));
}

TEST_CASE("XCBodyBuilder: clear") {
    XCBodyBuilder builder;

    // We want to see only sphere at the end
    builder.processCommand("cuboid 1 2 3");
    builder.clear();
    builder.processCommand("sphere 2");

    auto geom = builder.releaseCollideGeometry();
    visualize(*geom);

    CHECK_THAT(geom->getSupportPoint({2, 4, -7}), IsApproxEqual(2.*Vector<3>{2, 4, -7}.normalized(), 1e-12));
}
