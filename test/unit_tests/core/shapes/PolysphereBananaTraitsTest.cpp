//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolysphereBananaTraits.h"

TEST_CASE("PolysphereBananaTraits") {
    SECTION("odd number of spheres") {
        PolysphereBananaTraits traits(2, M_PI/2, 3, 1, false);

        SECTION("sphere data") {
            const auto &data = traits.getSphereData();
            REQUIRE(data.size() == 3);
            CHECK(data[0].radius == 1);
            CHECK(data[1].radius == 1);
            CHECK(data[2].radius == 1);
            CHECK_THAT(data[0].position, IsApproxEqual({2 - M_SQRT2, -M_SQRT2, 0}, 1e-12));
            CHECK_THAT(data[1].position, IsApproxEqual({0, 0, 0}, 1e-12));
            CHECK_THAT(data[2].position, IsApproxEqual({2 - M_SQRT2, +M_SQRT2, 0}, 1e-12));
        }

        SECTION("geometry") {
            const auto &geometry = traits.getGeometry();

            CHECK_THAT(geometry.getPrimaryAxis({}), IsApproxEqual({0, 1, 0}, 1e-12));
            CHECK_THAT(geometry.getSecondaryAxis({}), IsApproxEqual({-1, 0, 0}, 1e-12));
            CHECK_THAT(geometry.getGeometricOrigin({}), IsApproxEqual({4./3 - 2*M_SQRT2/3, 0, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForShape("s0", {}), IsApproxEqual({2 - M_SQRT2, -M_SQRT2, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForShape("beg", {}), IsApproxEqual({2 - M_SQRT2, -M_SQRT2, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForShape("s2", {}), IsApproxEqual({2 - M_SQRT2, +M_SQRT2, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForShape("end", {}), IsApproxEqual({2 - M_SQRT2, +M_SQRT2, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForShape("o", {}), IsApproxEqual({4. / 3 - 2 * M_SQRT2 / 3, 0, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForShape("cm", {}), IsApproxEqual({0, 0, 0}, 1e-12));
        }
    }

    SECTION("odd number of spheres (normalized)") {
        PolysphereBananaTraits traits(2, M_PI/2, 3, 1, true);

        const auto &data = traits.getSphereData();
        REQUIRE(data.size() == 3);
        CHECK_THAT(data[1].position, IsApproxEqual({-4./3 + 2*M_SQRT2/3, 0, 0}, 1e-12));
        CHECK(traits.getGeometry().getGeometricOrigin({}) == Vector<3>{}); // Strictly equal!
    }

    SECTION("even number of spheres") {
        PolysphereBananaTraits traits(2, 3*M_PI/4, 4, 1, false);

        const auto &data = traits.getSphereData();
        REQUIRE(data.size() == 4);
        CHECK(data[0].radius == 1);
        CHECK(data[1].radius == 1);
        CHECK(data[2].radius == 1);
        CHECK(data[3].radius == 1);
        CHECK_THAT(data[0].position, IsApproxEqual({2, -2, 0}, 1e-12));
        CHECK_THAT(data[1].position, IsApproxEqual({2 - M_SQRT2, -M_SQRT2, 0}, 1e-12));
        CHECK_THAT(data[2].position, IsApproxEqual({0, 0, 0}, 1e-12));
        CHECK_THAT(data[3].position, IsApproxEqual({2 - M_SQRT2, +M_SQRT2, 0}, 1e-12));
        CHECK_THAT(traits.getGeometry().getGeometricOrigin({}), IsApproxEqual({1.5-0.5*M_SQRT2, -0.5, 0}, 1e-12));
    }
}