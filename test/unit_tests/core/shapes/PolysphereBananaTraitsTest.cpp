//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolysphereBananaTraits.h"


TEST_CASE("PolysphereBananaTraits") {
    SECTION("below half-angle") {
        PolysphereBananaTraits traits(2, 2*M_PI/3, 3, 1);

        SECTION("spehre data") {
            const auto &sphereData = traits.getSphereData();
            REQUIRE(sphereData.size() == 3);
            CHECK_THAT(sphereData[0].position, IsApproxEqual({0, 0, -std::sqrt(3)}, 1e-12));
            CHECK_THAT(sphereData[1].position, IsApproxEqual({-1, 0, 0}, 1e-12));
            CHECK_THAT(sphereData[2].position, IsApproxEqual({0, 0, +std::sqrt(3)}, 1e-12));
            for (const auto &data: sphereData)
                CHECK(data.radius == 1);
        }

        SECTION("geometry") {
            const auto &geom = traits.getGeometry();
            CHECK_THAT(geom.getPrimaryAxis({}), IsApproxEqual({0, 0, 1}, 1e-12));
            CHECK_THAT(geom.getSecondaryAxis({}), IsApproxEqual({-1, 0, 0}, 1e-12));
            CHECK_THAT(geom.getNamedPoint("beg"), IsApproxEqual({0, 0, -std::sqrt(3)}, 1e-12));
            CHECK_THAT(geom.getNamedPoint("end"), IsApproxEqual({0, 0, +std::sqrt(3)}, 1e-12));
        }
    }

    SECTION("above half-angle") {
        PolysphereBananaTraits traits(2, 4*M_PI/3, 3, 1);

        const auto &sphereData = traits.getSphereData();
        REQUIRE(sphereData.size() == 3);
        CHECK_THAT(sphereData[0].position, IsApproxEqual({1, 0, -std::sqrt(3)}, 1e-12));
        CHECK_THAT(sphereData[1].position, IsApproxEqual({-2, 0, 0}, 1e-12));
        CHECK_THAT(sphereData[2].position, IsApproxEqual({1, 0, +std::sqrt(3)}, 1e-12));
        for (const auto &data : sphereData)
            CHECK(data.radius == 1);
    }
}