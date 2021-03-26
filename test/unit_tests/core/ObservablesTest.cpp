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
    Shape s3({4, 3, 1}, Matrix<3, 3>::rotation(0, 0, M_PI/4));
    Packing packing({3, 4, 5}, {s1, s2, s3}, std::move(pbc), mockShapeTraits.getInteraction(), 1, 1);

    SECTION("BoxDimensions") {
        BoxDimensions boxDimensions;

        boxDimensions.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(boxDimensions.getHeader() == std::vector<std::string>{"L_X", "L_Y", "L_Z"});
        CHECK(boxDimensions.getName() == "box dimensions");
        CHECK(boxDimensions.getValues() == std::vector<double>{3, 4, 5});
    }

    SECTION("CompressibilityFactor") {
        CompressibilityFactor compressibilityFactor;

        compressibilityFactor.calculate(packing, 4, 2, mockShapeTraits);

        CHECK(compressibilityFactor.getHeader() == std::vector<std::string>{"Z"});
        CHECK(compressibilityFactor.getName() == "compressibility factor");
        CHECK(compressibilityFactor.getValues() == std::vector<double>{10});
    }

    SECTION("EnergyFluctuationsPerParticle") {
        EnergyFluctuationsPerParticle energyFluctuationsPerParticle;

        energyFluctuationsPerParticle.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(energyFluctuationsPerParticle.getHeader() == std::vector<std::string>{"varE"});
        CHECK(energyFluctuationsPerParticle.getName() == "energy fluctuations per particle");
        // It is already tested in the Packing test
        CHECK(energyFluctuationsPerParticle.getValues()
              == std::vector<double>{packing.getParticleEnergyFluctuations(mockShapeTraits.getInteraction())});
    }

    SECTION("EnergyPerParticle") {
        EnergyPerParticle energyPerParticle;

        energyPerParticle.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(energyPerParticle.getHeader() == std::vector<std::string>{"E"});
        CHECK(energyPerParticle.getName() == "energy per particle");
        // It is already tested in the Packing test
        CHECK(energyPerParticle.getValues()
              == std::vector<double>{packing.getTotalEnergy(mockShapeTraits.getInteraction()) / 3});
    }

    SECTION("NematicOrder") {
        NematicOrder nematicOrder;

        nematicOrder.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(nematicOrder.getHeader() == std::vector<std::string>{"P2"});
        CHECK(nematicOrder.getName() == "nematic order");
        REQUIRE(nematicOrder.getValues().size() == 1);
        CHECK(nematicOrder.getValues()[0] == Approx(0.6403882032022076));   // Mathematica
    }

    SECTION("NumberDensity") {
        NumberDensity numberDensity;

        numberDensity.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(numberDensity.getHeader() == std::vector<std::string>{"rho"});
        CHECK(numberDensity.getName() == "number density");
        REQUIRE(numberDensity.getValues().size() == 1);
        CHECK(numberDensity.getValues()[0] == Approx(0.05));
    }

    SECTION("PackingFraction") {
        PackingFraction packingFraction;

        packingFraction.calculate(packing, 1, 1, mockShapeTraits);

        CHECK(packingFraction.getHeader() == std::vector<std::string>{"theta"});
        CHECK(packingFraction.getName() == "packing fraction");
        REQUIRE(packingFraction.getValues().size() == 1);
        CHECK(packingFraction.getValues()[0] == Approx(0.1));
    }
}