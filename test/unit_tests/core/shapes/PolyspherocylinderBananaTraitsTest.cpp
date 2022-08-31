//
// Created by Piotr Kubala on 27/04/2021.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolyspherocylinderBananaTraits.h"


TEST_CASE("PolyspherocylinderBananaTraits") {
    SECTION("without subdivisions") {
        PolyspherocylinderBananaTraits traits(2, M_PI, 2, 1, 1, false);

        SECTION("sphere data") {
            const auto &data = traits.getSpherocylinderData();
            REQUIRE(data.size() == 2);
            CHECK(data[0].radius == 1);
            CHECK(data[1].radius == 1);
            CHECK_THAT(data[0].position, IsApproxEqual({-1, 1, 0}, 1e-12));
            CHECK_THAT(data[0].halfAxis, IsApproxEqual({-1, -1, 0}, 1e-12));
            CHECK_THAT(data[1].position, IsApproxEqual({-1, -1, 0}, 1e-12));
            CHECK_THAT(data[1].halfAxis, IsApproxEqual({1, -1, 0}, 1e-12));
        }

        SECTION("geometry") {
            const auto &geometry = traits.getGeometry();

            CHECK_THAT(geometry.getPrimaryAxis({}), IsApproxEqual({0, 1, 0}, 1e-12));
            CHECK_THAT(geometry.getSecondaryAxis({}), IsApproxEqual({-1, 0, 0}, 1e-12));
            CHECK_THAT(geometry.getGeometricOrigin({}), IsApproxEqual({-1, 0, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPoint("o0", {}), IsApproxEqual({-1, 1, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPoint("b0", {}), IsApproxEqual({0, 2, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPoint("beg", {}), IsApproxEqual({0, 2, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPoint("e1", {}), IsApproxEqual({0, -2, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPoint("end", {}), IsApproxEqual({0, -2, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPoint("o", {}), IsApproxEqual({-1, 0, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPoint("cm", {}), IsApproxEqual({0, 0, 0}, 1e-12));
        }
    }

    SECTION("without subdivisions normalized") {
        PolyspherocylinderBananaTraits traits(2, M_PI, 2, 1, 1, true);

        const auto &data = traits.getSpherocylinderData();
        REQUIRE(data.size() == 2);
        CHECK(data[0].radius == 1);
        CHECK(data[1].radius == 1);
        CHECK_THAT(data[0].position, IsApproxEqual({0, 1, 0}, 1e-12));
        CHECK_THAT(data[0].halfAxis, IsApproxEqual({-1, -1, 0}, 1e-12));
        CHECK_THAT(data[1].position, IsApproxEqual({0, -1, 0}, 1e-12));
        CHECK_THAT(data[1].halfAxis, IsApproxEqual({1, -1, 0}, 1e-12));
        CHECK(traits.getGeometry().getGeometricOrigin({}) == Vector<3>{0, 0, 0});
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
        CHECK_THAT(traits.getGeometry().getGeometricOrigin({}), IsApproxEqual({-1, 0, 0}, 1e-12));
    }
}