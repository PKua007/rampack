//
// Created by pkup on 18.06.2021.
//

#include <catch2/catch.hpp>

#include "core/MinimalDistanceOptimizer.h"

#include "core/shapes/SpherocylinderTraits.h"

TEST_CASE("MinimalDiscanceOptimizer") {
    SpherocylinderTraits scTraits(2, 1);
    Shape sc1, sc2;
    sc2.setOrientation(Matrix<3, 3>::rotation(0, M_PI/2, 0));

    SECTION("forDirection: x") {
        double distance = MinimalDistanceOptimizer::forDirection(sc1, sc2, {1, 0, 0}, scTraits.getInteraction());

        CHECK(distance == Approx(3).margin(MinimalDistanceOptimizer::EPSILON));
    }

    SECTION("forAxes") {
        auto distances = MinimalDistanceOptimizer::forAxes(sc1, sc2, scTraits.getInteraction());

        CHECK(distances[0] == Approx(3).margin(MinimalDistanceOptimizer::EPSILON));
        CHECK(distances[1] == Approx(2).margin(MinimalDistanceOptimizer::EPSILON));
        CHECK(distances[2] == Approx(3).margin(MinimalDistanceOptimizer::EPSILON));
    }
}