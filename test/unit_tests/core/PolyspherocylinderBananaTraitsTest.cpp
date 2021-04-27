//
// Created by Piotr Kubala on 27/04/2021.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolyspherocylinderBananaTraits.h"


TEST_CASE("PolyspherocylinderBananaTraits") {
    SECTION("without subdivisions") {
        PolyspherocylinderBananaTraits traits(2, M_PI, 2, 1, 1, false);

        const auto &data = traits.getSpherocylinderData();
        REQUIRE(data.size() == 2);
        CHECK(data[0].radius == 1);
        CHECK(data[1].radius == 1);
        CHECK_THAT(data[0].position, IsApproxEqual({-1, 1, 0}, 1e-12));
        CHECK_THAT(data[0].halfAxis, IsApproxEqual({-1, -1, 0}, 1e-12));
        CHECK_THAT(data[1].position, IsApproxEqual({-1, -1, 0}, 1e-12));
        CHECK_THAT(data[1].halfAxis, IsApproxEqual({1, -1, 0}, 1e-12));
    }

    SECTION("with subdivisions") {
        PolyspherocylinderBananaTraits traits(2, M_PI, 2, 1, 2, false);

        const auto &data = traits.getSpherocylinderData();
        REQUIRE(data.size() == 4);
        CHECK(data[0].radius == 1);
        CHECK(data[1].radius == 1);
        CHECK(data[2].radius == 1);
        CHECK(data[3].radius == 1);
        CHECK_THAT(data[0].position, IsApproxEqual({-0.5, 1.5, 0}, 1e-12));
        CHECK_THAT(data[0].halfAxis, IsApproxEqual({-0.5, -0.5, 0}, 1e-12));
        CHECK_THAT(data[1].position, IsApproxEqual({-1.5, 0.5, 0}, 1e-12));
        CHECK_THAT(data[1].halfAxis, IsApproxEqual({-0.5, -0.5, 0}, 1e-12));
        CHECK_THAT(data[2].position, IsApproxEqual({-1.5, -0.5, 0}, 1e-12));
        CHECK_THAT(data[2].halfAxis, IsApproxEqual({0.5, -0.5, 0}, 1e-12));
        CHECK_THAT(data[3].position, IsApproxEqual({-0.5, -1.5, 0}, 1e-12));
        CHECK_THAT(data[3].halfAxis, IsApproxEqual({0.5, -0.5, 0}, 1e-12));
    }
}