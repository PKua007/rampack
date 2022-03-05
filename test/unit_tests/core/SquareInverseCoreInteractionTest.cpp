//
// Created by pkua on 05.03.2022.
//

#include <catch2/catch.hpp>

#include "core/interactions/SquareInverseCoreInteraction.h"
#include "core/FreeBoundaryConditions.h"

TEST_CASE("SquareInverseCoreInteraction") {
    SquareInverseCoreInteraction interaction(3, 2);
    Shape shape1({1, 0, 0}), shape2({2, 0, 0});
    Shape shape3({1, 0, 0}), shape4({4, 0, 0});
    FreeBoundaryConditions fbc;

    CHECK(interaction.calculateEnergyBetweenShapes(shape1, shape2, fbc) == Approx(9));
    CHECK(interaction.calculateEnergyBetweenShapes(shape3, shape4, fbc) == 0);
}