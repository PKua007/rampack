//
// Created by Piotr Kubala on 27/03/2021.
//

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeTraits.h"

#include "core/ObservablesCollector.h"

#include "core/observables/BoxDimensions.h"
#include "core/observables/CompressibilityFactor.h"
#include "core/observables/NumberDensity.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("ObservablesCollector") {
    using trompeloeil::_;

    MockShapeTraits mockShapeTraits;
    ALLOW_CALL(mockShapeTraits, getRangeRadius()).RETURN(std::numeric_limits<double>::infinity());
    ALLOW_CALL(mockShapeTraits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(mockShapeTraits, getTotalRangeRadius()).RETURN(std::numeric_limits<double>::infinity());
    ALLOW_CALL(mockShapeTraits, hasSoftPart()).RETURN(true);
    ALLOW_CALL(mockShapeTraits, hasHardPart()).RETURN(false);
    ALLOW_CALL(mockShapeTraits, calculateEnergyBetween(_, _, _, _, _, _, _)).RETURN(0);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Shape s1({1, 1, 1});
    Shape s2({2, 3, 3});
    Shape s3({4, 3, 1});
    Packing packing({3, 4, 5}, {s1, s2, s3}, std::move(pbc), mockShapeTraits.getInteraction(), 1, 1);

    ObservablesCollector collector;
    collector.addObservable(std::make_unique<BoxDimensions>(), true);
    collector.addObservable(std::make_unique<CompressibilityFactor>(), false);
    collector.addObservable(std::make_unique<NumberDensity>(), true);
    collector.setThermodynamicParameters(4, 2);

    SECTION("snapshots") {
        collector.addSnapshot(packing, 100, mockShapeTraits);
        packing.tryScaling(2, mockShapeTraits.getInteraction());
        collector.addSnapshot(packing, 200, mockShapeTraits);
        std::ostringstream out;

        collector.printSnapshots(out);

        CHECK(out.str() == "cycle L_X L_Y L_Z Z rho \n"
                           "100 3 4 5 10 0.05 \n"
                           "200 6 8 10 80 0.00625 \n");
    }

    SECTION("average values") {
        collector.addAveragingValues(packing, mockShapeTraits);
        packing.tryScaling(2, mockShapeTraits.getInteraction());
        collector.addAveragingValues(packing, mockShapeTraits);

        SECTION("flattened") {
            auto values = collector.getFlattenedAverageValues();

            REQUIRE(values.size() == 5);
            CHECK(values[0].name == "L_X");
            CHECK(values[0].value.value == Approx(4.5));
            CHECK(values[1].name == "L_Y");
            CHECK(values[1].value.value == Approx(6));
            CHECK(values[2].name == "L_Z");
            CHECK(values[2].value.value == Approx(7.5));
            CHECK(values[3].name == "Z");
            CHECK(values[3].value.value == Approx(45));
            CHECK(values[4].name == "rho");
            CHECK(values[4].value.value == Approx(0.028125));
        }

        SECTION("grouped") {
            auto values = collector.getGroupedAverageValues();

            REQUIRE(values.size() == 3);
            CHECK(values[0].groupName == "box dimensions");

            REQUIRE(values[0].observableData.size() == 3);
            CHECK(values[0].observableData[0].name == "L_X");
            CHECK(values[0].observableData[0].value.value == Approx(4.5));
            CHECK(values[0].observableData[1].name == "L_Y");
            CHECK(values[0].observableData[1].value.value == Approx(6));
            CHECK(values[0].observableData[2].name == "L_Z");
            CHECK(values[0].observableData[2].value.value == Approx(7.5));

            REQUIRE(values[1].observableData.size() == 1);
            CHECK(values[1].observableData[0].name == "Z");
            CHECK(values[1].observableData[0].value.value == Approx(45));

            REQUIRE(values[2].observableData.size() == 1);
            CHECK(values[2].observableData[0].name == "rho");
            CHECK(values[2].observableData[0].value.value == Approx(0.028125));
        }
    }

    SECTION("inline string") {
        std::string inlineString = collector.generateInlineObservablesString(packing, mockShapeTraits);

        CHECK(inlineString == "L_X: 3, L_Y: 4, L_Z: 5, rho: 0.05");
    }
}