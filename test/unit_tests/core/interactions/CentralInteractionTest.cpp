//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "core/interactions/CentralInteraction.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    class DummyInteraction : public CentralInteraction<DummyInteraction, double> {
    public:
        [[nodiscard]] double calculateEnergyForDistance2(double distance2,
                                                         [[maybe_unused]] const double &pairData) const {
            return std::sqrt(distance2);
        }
        [[nodiscard]] static double getRangeRadiusForPairData([[maybe_unused]] const double &pairData) { return 0; }
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

        interaction.bindCentreLayout({{{0, 0, 0}, {2, 0, 0}}, {0, 0}});

        REQUIRE(interaction.getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}, {2, 0, 0}});
    }

    SECTION("sphere constructor") {
        DummyInteraction interaction;

        REQUIRE(interaction.getInteractionCentres().empty());
    }
}

TEST_CASE("CentralInteraction: calculating energy") {
    DummyInteraction interaction;
    Shape shape1({1, 5, 5});
    Shape shape2({9, 5, 5});
    PeriodicBoundaryConditions pbc(10);

    CHECK(interaction.calculateEnergyBetweenShapes(shape1, shape2, pbc) == Approx(2));
}
