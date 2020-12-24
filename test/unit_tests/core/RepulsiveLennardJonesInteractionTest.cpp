//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "core/interactions/RepulsiveLennardJonesInteraction.h"
#include "core/FreeBoundaryConditions.h"

TEST_CASE("RepulsiveLennardJonesInteraction") {
    RepulsiveLennardJonesInteraction interaction(3, 2);
    FreeBoundaryConditions fbc;

    SECTION("repulsive part") {
        Shape shape1({1, 0, 0}), shape2({3, 0, 0});

        CHECK(interaction.calculateEnergyBetween(shape1, shape2, fbc) == Approx(3));
    }

    SECTION("free part") {
        Shape shape1({1, 0, 0}), shape2({5, 0, 0});

        CHECK(interaction.calculateEnergyBetween(shape1, shape2, fbc) == Approx(0).margin(1e-12));
    }
}