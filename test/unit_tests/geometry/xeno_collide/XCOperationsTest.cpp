//
// Created by pkua on 14.10.22.
//

#include <catch2/catch.hpp>

#include "geometry/xenocollide/XCOperations.h"
#include "geometry/xenocollide/XCPrimitives.h"


TEST_CASE("XCOperations: XCSum") {
    auto s1 = std::make_shared<XCEllipsoid>(Vector<3>{1, 2, 3});
    auto s2 = std::make_shared<XCEllipsoid>(Vector<3>{2, 3, 5});

    XCSum sum(s1, {-0.5, 0, 0}, s2, {0.3, 0, 0});

    CHECK(sum.getInsphereRadius() == Approx(2.8));
    CHECK(sum.getCircumsphereRadius() == Approx(8.2));
}

TEST_CASE("XCOperations: XCDiff") {
    auto s1 = std::make_shared<XCEllipsoid>(Vector<3>{1, 2, 3});
    auto s2 = std::make_shared<XCEllipsoid>(Vector<3>{2, 3, 5});

    XCDiff diff(s1, {-0.5, 0, 0}, s2, {0.3, 0, 0});

    CHECK(diff.getInsphereRadius() == Approx(2.2));
    CHECK(diff.getCircumsphereRadius() == Approx(8.8));
}

TEST_CASE("XCOperations: XCMax") {
    auto s1 = std::make_shared<XCEllipsoid>(Vector<3>{1, 2, 3});
    auto s2 = std::make_shared<XCEllipsoid>(Vector<3>{2, 3, 5});

    XCMax diff(s1, {-1.5, -0.1, 0}, s2, {0.5, -0.1, 0});

    CHECK(diff.getInsphereRadius() == Approx(0.75*2 + 0.25*1 - 0.1));
    CHECK(diff.getCircumsphereRadius() == Approx(std::sqrt(0.5*0.5 + 0.1*0.1) + 5));
}