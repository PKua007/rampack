//
// Created by pkua on 01.03.2022.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolysphereLollipopTraits.h"


TEST_CASE("PolysphereLollipopTraits") {
    std::size_t sphereNum = 3;
    double smallSphereRadius = 2;
    double largeSphereRadius = 4;
    double smallSpherePenetration = 1;
    double largeSpherePenetration = 2;
    PolysphereLollipopTraits traits(sphereNum, smallSphereRadius, largeSphereRadius, smallSpherePenetration,
                                    largeSpherePenetration);

    SECTION("sphere data") {
        const auto &data = traits.getSphereData();
        REQUIRE(data.size() == 3);
        CHECK(data[0].radius == 2);
        CHECK(data[1].radius == 2);
        CHECK(data[2].radius == 4);
        CHECK_THAT(data[0].position, IsApproxEqual({0, 0, -4.5}, 1e-12));
        CHECK_THAT(data[1].position, IsApproxEqual({0, 0, -1.5}, 1e-12));
        CHECK_THAT(data[2].position, IsApproxEqual({0, 0, 2.5}, 1e-12));
    }

    SECTION("geometry") {
        const auto &geometry = traits.getGeometry();
        CHECK_THAT(geometry.getGeometricOrigin({}), IsApproxEqual({0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getPrimaryAxis({}), IsApproxEqual({0, 0, 1}, 1e-12));
        CHECK_THAT(geometry.getSecondaryAxis({}), IsApproxEqual({1, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("s0"), IsApproxEqual({0, 0, -4.5}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("ss"), IsApproxEqual({0, 0, -4.5}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("s2"), IsApproxEqual({0, 0, 2.5}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("sl"), IsApproxEqual({0, 0, 2.5}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("o"), IsApproxEqual({0, 0, 0}, 1e-12));
    }
}