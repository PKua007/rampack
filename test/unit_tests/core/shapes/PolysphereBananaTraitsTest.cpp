//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolysphereBananaTraits.h"


namespace {
    const Shape defaultShape({}, Matrix<3, 3>::identity(), PolysphereTraits::Data{0});
}

TEST_CASE("PolysphereBananaTraits: basics") {
    SECTION("below half-angle") {
        PolysphereBananaTraits traits(2, 2*M_PI/3, 3, 1);

        SECTION("spehre data") {
            const auto &sphereData = traits.getDefaultShape().getSphereData();
            REQUIRE(sphereData.size() == 3);
            CHECK_THAT(sphereData[0].position, IsApproxEqual({0, 0, -std::sqrt(3)}, 1e-12));
            CHECK_THAT(sphereData[1].position, IsApproxEqual({-1, 0, 0}, 1e-12));
            CHECK_THAT(sphereData[2].position, IsApproxEqual({0, 0, +std::sqrt(3)}, 1e-12));
            for (const auto &data: sphereData)
                CHECK(data.radius == 1);
        }

        SECTION("geometry") {
            const auto &geom = traits.getGeometry();
            CHECK_THAT(geom.getPrimaryAxis(defaultShape), IsApproxEqual({0, 0, 1}, 1e-12));
            CHECK_THAT(geom.getSecondaryAxis(defaultShape), IsApproxEqual({-1, 0, 0}, 1e-12));
            CHECK_THAT(geom.getNamedPoint("beg").forShape(defaultShape), IsApproxEqual({0, 0, -std::sqrt(3)}, 1e-12));
            CHECK_THAT(geom.getNamedPoint("end").forShape(defaultShape), IsApproxEqual({0, 0, +std::sqrt(3)}, 1e-12));
            // "cm" named point has a separate test
        }
    }

    SECTION("above half-angle") {
        PolysphereBananaTraits traits(2, 4*M_PI/3, 3, 1);

        const auto &sphereData = traits.getDefaultShape().getSphereData();
        REQUIRE(sphereData.size() == 3);
        CHECK_THAT(sphereData[0].position, IsApproxEqual({1, 0, -std::sqrt(3)}, 1e-12));
        CHECK_THAT(sphereData[1].position, IsApproxEqual({-2, 0, 0}, 1e-12));
        CHECK_THAT(sphereData[2].position, IsApproxEqual({1, 0, +std::sqrt(3)}, 1e-12));
        for (const auto &data : sphereData)
            CHECK(data.radius == 1);
    }
}

TEST_CASE("PolysphereBananaTraits: mass centre") {
    SECTION("existing") {
        PolysphereBananaTraits traits(3, M_PI, 3, 0.5);

        CHECK_THAT(traits.getGeometry().getNamedPoint("cm").forShape(defaultShape), IsApproxEqual({-1, 0, 0}, 1e-12));
    }

    SECTION("not existing") {
        PolysphereBananaTraits traits(3, 0.1, 3, 0.5);

        CHECK_FALSE(traits.getGeometry().hasNamedPoint("cm"));
    }
}

TEST_CASE("PolysphereBananaTraits: volume") {
    SECTION("non-overlapping") {
        PolysphereBananaTraits traits(M_SQRT1_2, M_PI, 3, 0.5);
        CHECK(traits.getGeometry().getVolume(defaultShape) == Approx(3*4./3*M_PI*0.5*0.5*0.5));
    }

    SECTION("overlapping below half-circle") {
        PolysphereBananaTraits traits(2, M_PI/3, 3, 1);
        CHECK(traits.getGeometry().getVolume(defaultShape) == Approx(4./3*M_PI*(1 + M_SQRT2)));
    }

    SECTION("overlapping over half-circle") {
        SECTION("not overlapping ends") {
            PolysphereBananaTraits traits(3, 4*M_PI/3, 5, 2);
            CHECK(traits.getGeometry().getVolume(defaultShape) == Approx(156.032435128293));
        }

        SECTION("overlapping ends") {
            PolysphereBananaTraits traits(3, 7*M_PI/4, 5, 2.5);
            CHECK(traits.getGeometry().getVolume(defaultShape) == Approx(283.1146697687217));
        }

        SECTION("overlapping ends for 2 spheres") {
            PolysphereBananaTraits traits(3, 7*M_PI/4, 2, 2);
            CHECK(traits.getGeometry().getVolume(defaultShape) == Approx(59.19483315048983));
        }
    }
}