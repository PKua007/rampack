//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "core/interactions/CentralInteraction.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    class DummyInteraction : public CentralInteraction {
    protected:
        [[nodiscard]] double calculateEnergyForDistance2(double distance2) const override {
            return std::sqrt(distance2);
        }
    };
}

TEST_CASE("CentralInteraction: basics") {
    DummyInteraction interaction;

    CHECK(interaction.hasSoftPart());
    CHECK_FALSE(interaction.hasHardPart());
}

TEST_CASE("CentralInteraction: point installation") {
    SECTION("installOnCentres") {
        DummyInteraction interaction;

        interaction.installOnCentres({{0, 0, 0}, {2, 0, 0}});

        REQUIRE(interaction.getInteractionCentres(nullptr) == std::vector<Vector<3>>{{0, 0, 0}, {2, 0, 0}});
    }

    SECTION("installOnSphere") {
        DummyInteraction interaction;

        interaction.installOnSphere();

        REQUIRE(interaction.getInteractionCentres(nullptr).empty());
    }
}

TEST_CASE("CentralInteraction: calculating energy") {
    DummyInteraction interaction;
    interaction.installOnSphere();
    Shape shape1({1, 5, 5});
    Shape shape2({9, 5, 5});
    PeriodicBoundaryConditions pbc(10);

    CHECK(interaction.calculateEnergyBetweenShapes(shape1, shape2, pbc) == Approx(2));
}