//
// Created by Piotr Kubala on 27/04/2021.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolyspherocylinderBananaTraits.h"


TEST_CASE("PolyspherocylinderBananaTraits: validation") {
    SECTION("origin outside") {
        // 2 spherocylinders on equilateral triangle sides; one case barely misses triangle's origin, the second one
        // barely touches it
        double side = 2;
        double h = side * std::sqrt(3) / 2;
        double epsilon = 0.001;
        CHECK(PolyspherocylinderBananaTraits::isArcOriginOutside(2*h/3, 4*M_PI/3, 2, h/3 - epsilon));
        CHECK_FALSE(PolyspherocylinderBananaTraits::isArcOriginOutside(2*h/3, 4*M_PI/3, 2, h/3 + epsilon));
    }

    SECTION("arc open") {
        SECTION("hexagon") {
            double epsilon = 0.001;
            CHECK(PolyspherocylinderBananaTraits::isArcOpen(2, 5 * M_PI / 3, 5, 1 - epsilon));
            CHECK_FALSE(PolyspherocylinderBananaTraits::isArcOpen(2, 5 * M_PI / 3, 5, 1 + epsilon));
        }

        SECTION("almost straight") {
            CHECK(PolyspherocylinderBananaTraits::isArcOpen(2, M_PI/3, 3, 1.5));
        }
    }
}

TEST_CASE("PolyspherocylinderBananaTraits: points") {
    SECTION("below half-angle") {
        SECTION("without subdivisions") {
            PolyspherocylinderBananaTraits traits(2, 2*M_PI/3, 2, 1);

            SECTION("spehre data") {
                const auto &scData = traits.getSpherocylinderData();
                REQUIRE(scData.size() == 2);
                CHECK_THAT(scData[0].position, IsApproxEqual({-0.5, 0, -0.5*std::sqrt(3)}, 1e-12));
                CHECK_THAT(scData[0].halfAxis, IsApproxEqual({-0.5, 0, +0.5*std::sqrt(3)}, 1e-12));
                CHECK(scData[0].radius == 1);
                CHECK_THAT(scData[1].position, IsApproxEqual({-0.5, 0, +0.5*std::sqrt(3)}, 1e-12));
                CHECK_THAT(scData[1].halfAxis, IsApproxEqual({+0.5, 0, +0.5*std::sqrt(3)}, 1e-12));
                CHECK(scData[1].radius == 1);
            }

            SECTION("interaction") {
                const auto &interaction = traits.getInteraction();
                CHECK(interaction.getRangeRadius(nullptr) == Approx(4));
                CHECK(interaction.getTotalRangeRadius(nullptr) == Approx(6));
            }

            SECTION("geometry") {
                const auto &geom = traits.getGeometry();
                CHECK_THAT(geom.getPrimaryAxis({}), IsApproxEqual({0, 0, 1}, 1e-12));
                CHECK_THAT(geom.getSecondaryAxis({}), IsApproxEqual({-1, 0, 0}, 1e-12));
                CHECK_THAT(geom.getNamedPoint("beg"), IsApproxEqual({0, 0, -std::sqrt(3)}, 1e-12));
                CHECK_THAT(geom.getNamedPoint("end"), IsApproxEqual({0, 0, +std::sqrt(3)}, 1e-12));
            }
        }

        SECTION("with subdivisions") {
            PolyspherocylinderBananaTraits traits(2, 2*M_PI/3, 2, 1, 2);

            const auto &scData = traits.getSpherocylinderData();
            REQUIRE(scData.size() == 4);
            CHECK_THAT(scData[0].position, IsApproxEqual({-0.25, 0, -0.75*std::sqrt(3)}, 1e-12));
            CHECK_THAT(scData[0].halfAxis, IsApproxEqual({-0.25, 0, +0.25*std::sqrt(3)}, 1e-12));
            CHECK_THAT(scData[1].position, IsApproxEqual({-0.75, 0, -0.25*std::sqrt(3)}, 1e-12));
            CHECK_THAT(scData[1].halfAxis, IsApproxEqual({-0.25, 0, +0.25*std::sqrt(3)}, 1e-12));
            CHECK_THAT(scData[2].position, IsApproxEqual({-0.75, 0, +0.25*std::sqrt(3)}, 1e-12));
            CHECK_THAT(scData[2].halfAxis, IsApproxEqual({+0.25, 0, +0.25*std::sqrt(3)}, 1e-12));
            CHECK_THAT(scData[3].position, IsApproxEqual({-0.25, 0, +0.75*std::sqrt(3)}, 1e-12));
            CHECK_THAT(scData[3].halfAxis, IsApproxEqual({+0.25, 0, +0.25*std::sqrt(3)}, 1e-12));
            for (const auto &data : scData)
                CHECK(data.radius == 1);
        }
    }

    SECTION("above half-angle") {
        PolyspherocylinderBananaTraits traits(2, 4*M_PI/3, 2, 1);

        const auto &scData = traits.getSpherocylinderData();
        REQUIRE(scData.size() == 2);
        CHECK_THAT(scData[0].position, IsApproxEqual({-0.5, 0, -0.5*std::sqrt(3)}, 1e-12));
        CHECK_THAT(scData[0].halfAxis, IsApproxEqual({-1.5, 0, +0.5*std::sqrt(3)}, 1e-12));
        CHECK(scData[0].radius == 1);
        CHECK_THAT(scData[1].position, IsApproxEqual({-0.5, 0, +0.5*std::sqrt(3)}, 1e-12));
        CHECK_THAT(scData[1].halfAxis, IsApproxEqual({+1.5, 0, +0.5*std::sqrt(3)}, 1e-12));
        CHECK(scData[1].radius == 1);
    }
}


TEST_CASE("PolyspherocylinderBananaTraits: volume") {
    SECTION("2 acute segments") {
        PolyspherocylinderBananaTraits traits(3, 5*M_PI/3, 2, 1);
        CHECK(traits.getGeometry().getVolume({}) == Approx(37.3725974707442));    // Mathematica value
    }

    SECTION("3 obtuse segments") {
        PolyspherocylinderBananaTraits traits(2, M_PI/3, 3, 1);
        CHECK(traits.getGeometry().getVolume({}) == Approx(10.73038812797451));    // Mathematica value
    }
}