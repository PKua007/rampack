//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "core/interactions/CentralInteraction.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    class PairDataInteraction : public CentralInteraction<PairDataInteraction, double> {
    public:
        using CentralInteraction<PairDataInteraction, double>::CentralInteraction;

        [[nodiscard]] double calculateEnergyForDistance2(double distance2, const double &pairData) const
        {
            return pairData * std::sqrt(distance2);
        }

        [[nodiscard]] static double getRangeRadiusForPairData(const double &pairData) { return pairData; }
    };
}

TEST_CASE("CentralInteraction: basics") {
    PairDataInteraction interaction;

    CHECK(interaction.hasSoftPart());
    CHECK_FALSE(interaction.hasHardPart());
}

TEST_CASE("CentralInteraction: point installation") {
    SECTION("installOnCentres") {
        PairDataInteraction interaction;

        interaction.bindCentreLayout({{{0, 0, 0}, {2, 0, 0}}, {0, 0}});

        REQUIRE(interaction.getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}, {2, 0, 0}});
    }

    SECTION("sphere constructor") {
        PairDataInteraction interaction;

        REQUIRE(interaction.getInteractionCentres().empty());
    }
}

TEST_CASE("CentralInteraction: calculating energy") {
    PairDataInteraction interaction(3.5);
    Shape shape1({1, 5, 5});
    Shape shape2({9, 5, 5});
    PeriodicBoundaryConditions pbc(10);

    CHECK(interaction.calculateEnergyBetweenShapes(shape1, shape2, pbc) == Approx(7));
}

TEST_CASE("CentralInteraction: range radius") {
    SECTION("single pair data") {
        PairDataInteraction interaction(3.5);

        CHECK(interaction.getRangeRadius() == Approx(3.5));
    }

    SECTION("maximum over pair data map") {
        CentrePairDataMap<double> pairDataMap(3);
        pairDataMap.setPairData(0, 0, 1.5);
        pairDataMap.setPairData(0, 1, 4.0);
        pairDataMap.setPairData(0, 2, 2.0);
        pairDataMap.setPairData(1, 1, 3.0);
        pairDataMap.setPairData(1, 2, 5.5);
        pairDataMap.setPairData(2, 2, 0.5);
        PairDataInteraction interaction(pairDataMap);

        CHECK(interaction.getRangeRadius() == Approx(5.5));
    }
}

TEST_CASE("CentralInteraction: binding centre layout") {
    SECTION("broadcasts uniform pair data when enabled") {
        PairDataInteraction interaction(3.5);
        PeriodicBoundaryConditions pbc(100);
        const Matrix<3, 3> id = Matrix<3, 3>::identity();

        interaction.bindCentreLayout({{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}}, {2, 0, 1}}, true);

        CHECK(interaction.calculateEnergyBetween({0, 0, 0}, id, 0, {0, 0, 0}, id, 1, pbc) == Approx(0));
        CHECK(interaction.calculateEnergyBetween({0, 0, 0}, id, 0, {0, 1, 0}, id, 2, pbc) == Approx(3.5));
        CHECK(interaction.calculateEnergyBetween({0, 0, 0}, id, 1, {0, 4, 0}, id, 2, pbc) == Approx(14));
    }

    SECTION("fails without broadcast when layout has too many centre types") {
        PairDataInteraction interaction(3.5);

        CHECK_THROWS_AS(interaction.bindCentreLayout({{{0, 0, 0}, {1, 0, 0}}, {0, 1}}),
                        PreconditionException);
    }

    SECTION("fails when broadcast is requested for non-uniform pair data") {
        CentrePairDataMap<double> pairDataMap(2);
        pairDataMap.setPairData(0, 0, 1.0);
        pairDataMap.setPairData(0, 1, 2.0);
        pairDataMap.setPairData(1, 1, 3.0);
        PairDataInteraction interaction(pairDataMap);

        CHECK_THROWS_AS(interaction.bindCentreLayout({{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}}, {0, 1, 2}}, true),
                        PreconditionException);
    }
}
