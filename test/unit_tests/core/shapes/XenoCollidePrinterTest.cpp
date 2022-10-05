//
// Created by pkua on 04.10.22.
//

#include <catch2/catch.hpp>

#include "geometry/xenocollide/XCPrinter.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


TEST_CASE("XenoCollidePrinter") {
    double r = 1;
    double l = 2;
    XCBodyBuilder bb;
    bb.sphere(r);
    bb.move(-l/2, 0, 0);
    bb.sphere(r);
    bb.move(l/2, 0, 0);
    bb.wrap();
    auto geometry = bb.getCollideGeometry();

    auto polyhedron = XCPrinter::buildPolyhedron(*geometry, 5);

    double expectedVolume = M_PI*(4./3*r*r*r + r*r*l);
    CHECK(polyhedron.getVolume() == Approx(expectedVolume).margin(0.05));
}