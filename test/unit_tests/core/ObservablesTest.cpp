//
// Created by Piotr Kubala on 26/03/2021.
//

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeTraits.h"

#include "core/observables/BoxDimensions.h"
#include "core/observables/CompressibilityFactor.h"
#include "core/observables/EnergyFluctuationsPerParticle.h"
#include "core/observables/EnergyPerParticle.h"
#include "core/observables/NematicOrder.h"
#include "core/observables/NumberDensity.h"
#include "core/observables/PackingFraction.h"

#include "core/ShapeTraits.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("Observables") {
    using trompeloeil::_;

    MockShapeTraits mockShapeTraits;
    ALLOW_CALL(mockShapeTraits, getVolume()).RETURN(2);
    ALLOW_CALL(mockShapeTraits, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    ALLOW_CALL(mockShapeTraits, hasSoftPart()).RETURN(true);
    ALLOW_CALL(mockShapeTraits, hasHardPart()).RETURN(false);
    ALLOW_CALL(mockShapeTraits, calculateEnergyBetween(_, _, _, _, _, _, _)).RETURN((_1 - _4).norm());
    ALLOW_CALL(mockShapeTraits, getRangeRadius()).RETURN(std::numeric_limits<double>::infinity());
    ALLOW_CALL(mockShapeTraits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(mockShapeTraits, getTotalRangeRadius()).RETURN(std::numeric_limits<double>::infinity());
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Shape s1({1, 1, 1}, Matrix<3, 3>::rotation(M_PI/4, 0, 0));
    Shape s2({2, 3, 3}, Matrix<3, 3>::rotation(0, M_PI/4, 0));
    Shape s3({2.5, 3, 1}, Matrix<3, 3>::rotation(0, 0, M_PI/4));
    Packing packing({3, 4, 5}, {s1, s2, s3}, std::move(pbc), mockShapeTraits.getInteraction(), 1, 1);

    SECTION("BoxDimensions") {
        BoxDimensions boxDimensions;

        boxDimensions.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(boxDimensions.getIntervalHeader() == std::vector<std::string>{"L_X", "L_Y", "L_Z"});
        CHECK(boxDimensions.getName() == "box dimensions");
        CHECK(boxDimensions.getIntervalValues() == std::vector<double>{3, 4, 5});
    }

    SECTION("CompressibilityFactor") {
        CompressibilityFactor compressibilityFactor;

        compressibilityFactor.calculate(packing, 4, 2, mockShapeTraits);

        CHECK(compressibilityFactor.getIntervalHeader() == std::vector<std::string>{"Z"});
        CHECK(compressibilityFactor.getName() == "compressibility factor");
        CHECK(compressibilityFactor.getIntervalValues() == std::vector<double>{10});
    }

    SECTION("EnergyFluctuationsPerParticle") {
        EnergyFluctuationsPerParticle energyFluctuationsPerParticle;

        energyFluctuationsPerParticle.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(energyFluctuationsPerParticle.getIntervalHeader() == std::vector<std::string>{"varE"});
        CHECK(energyFluctuationsPerParticle.getName() == "energy fluctuations per particle");
        // It is already tested in the Packing test
        CHECK(energyFluctuationsPerParticle.getIntervalValues()
              == std::vector<double>{packing.getParticleEnergyFluctuations(mockShapeTraits.getInteraction())});
    }

    SECTION("EnergyPerParticle") {
        EnergyPerParticle energyPerParticle;

        energyPerParticle.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(energyPerParticle.getIntervalHeader() == std::vector<std::string>{"E"});
        CHECK(energyPerParticle.getName() == "energy per particle");
        // It is already tested in the Packing test
        CHECK(energyPerParticle.getIntervalValues()
              == std::vector<double>{packing.getTotalEnergy(mockShapeTraits.getInteraction()) / 3});
    }

    SECTION("NematicOrder") {
        SECTION("positive order with Q-tensor dump") {
            NematicOrder nematicOrder(true);

            nematicOrder.calculate(packing, 1, 1, mockShapeTraits);

            CHECK(nematicOrder.getIntervalHeader()
                  == std::vector<std::string>{"P2", "Q_11", "Q_12", "Q_13", "Q_22", "Q_23", "Q_33"});
            CHECK(nematicOrder.getName() == "nematic order");
            REQUIRE(nematicOrder.getIntervalValues().size() == 7);
            CHECK(nematicOrder.getIntervalValues()[0] == Approx(0.6403882032022076)); // Mathematica
            CHECK(nematicOrder.getIntervalValues()[1] == Approx(0.5));
            CHECK(nematicOrder.getIntervalValues()[2] == Approx(0.25));
            CHECK(nematicOrder.getIntervalValues()[3] == Approx(-0.25));
            CHECK(nematicOrder.getIntervalValues()[4] == Approx(-0.25));
            CHECK(nematicOrder.getIntervalValues()[5] == Approx(0));
            CHECK(nematicOrder.getIntervalValues()[6] == Approx(-0.25));
        }

        SECTION("negative order without Q-tensor dump") {
            NematicOrder nematicOrder;
            Shape s2_1({1, 1, 1}, Matrix<3, 3>::rotation(0, M_PI/2, 0));
            Shape s2_2({2, 3, 3}, Matrix<3, 3>::rotation(0, -2*M_PI/3, 0));
            Shape s2_3({2.5, 3, 1}, Matrix<3, 3>::rotation(0, 0, M_PI/2));
            Shape s2_4({2, 3, 2}, Matrix<3, 3>::rotation(0, 0, -2*M_PI/3));
            auto pbc2 = std::make_unique<PeriodicBoundaryConditions>();
            Packing packing2({3, 4, 5}, {s2_1, s2_2, s2_3, s2_4}, std::move(pbc2), mockShapeTraits.getInteraction(), 1,
                             1);

            nematicOrder.calculate(packing2, 1, 1, mockShapeTraits);

            REQUIRE(nematicOrder.getIntervalValues().size() == 1);
            CHECK(nematicOrder.getIntervalValues()[0] == Approx(-13./32));   // Mathematica
        }
    }

    SECTION("NumberDensity") {
        NumberDensity numberDensity;

        numberDensity.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(numberDensity.getIntervalHeader() == std::vector<std::string>{"rho"});
        CHECK(numberDensity.getName() == "number density");
        REQUIRE(numberDensity.getIntervalValues().size() == 1);
        CHECK(numberDensity.getIntervalValues()[0] == Approx(0.05));
    }

    SECTION("PackingFraction") {
        PackingFraction packingFraction;

        packingFraction.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(packingFraction.getIntervalHeader() == std::vector<std::string>{"theta"});
        CHECK(packingFraction.getName() == "packing fraction");
        REQUIRE(packingFraction.getIntervalValues().size() == 1);
        CHECK(packingFraction.getIntervalValues()[0] == Approx(0.1));
    }
}