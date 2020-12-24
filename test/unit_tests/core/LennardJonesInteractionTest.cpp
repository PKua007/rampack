//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "core/interactions/LennardJonesInteraction.h"
#include "core/FreeBoundaryConditions.h"

TEST_CASE("LennardJonesInteraction") {
    LennardJonesInteraction interaction(3, 2);
    Shape shape1({1, 0, 0}), shape2({5, 0, 0});
    FreeBoundaryConditions fbc;

    CHECK(interaction.calculateEnergyBetween(shape1, shape2, fbc) == Approx(-0.1845703125000000));
}