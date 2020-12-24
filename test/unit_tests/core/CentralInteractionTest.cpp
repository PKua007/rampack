//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "core/interactions/CentralInteraction.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    class DummyInteraction : public CentralInteraction {
    protected:
        [[nodiscard]] double calculateEnergyForDistance(double distance) const override { return distance; }
    };
}

TEST_CASE("CentralInteraction: basics") {
    DummyInteraction interaction;

    CHECK(interaction.hasSoftPart());
    CHECK_FALSE(interaction.hasHardPart());
}

TEST_CASE("CentralInteraction: multiple points") {
    // Particles look and are placed like this (x - central particle, o - second one):
    //
    //        2   4   6   8
    //   ||   x-->o   o<--x  ||
    //
    // Distances through BC: d(2, 6) = 4, d(2, 8) = 4, d(4, 6) = 2, d(4, 8) = 4
    // Each particle is reflected separately!

    DummyInteraction interaction;
    interaction.installOnCentres({{0, 0, 0}, {2, 0, 0}});
    Shape shape1({1, 5, 5}, Matrix<3, 3>::identity());
    Shape shape2({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0));
    PeriodicBoundaryConditions pbc(10);

    CHECK(interaction.calculateEnergyBetween(shape1, shape2, pbc) == Approx(14));
}

TEST_CASE("CentralInteraction: sphere case") {
    DummyInteraction interaction;
    interaction.installOnSphere();
    Shape shape1({1, 5, 5});
    Shape shape2({9, 5, 5});
    PeriodicBoundaryConditions pbc(10);

    CHECK(interaction.calculateEnergyBetween(shape1, shape2, pbc) == Approx(2));
}