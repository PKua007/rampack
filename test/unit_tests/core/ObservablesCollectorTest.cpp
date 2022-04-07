//
// Created by Piotr Kubala on 27/03/2021.
//

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeTraits.h"
#include "mocks/MockObservable.h"

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

    std::array<double, 3> boxSize{0, 0, 0};
    auto dimFormatter = [&boxSize]() {
        std::ostringstream out;
        out << boxSize[0] << "x" << boxSize[1] << "x" << boxSize[2];
        return out.str();
    };
    auto mockObservable = std::make_unique<MockObservable>();
    ALLOW_CALL(*mockObservable, calculate(_, _, _, _)).LR_SIDE_EFFECT(boxSize = _1.getBox().getHeights());
    ALLOW_CALL(*mockObservable, getIntervalHeader()).RETURN(std::vector<std::string>{"L_X", "L_Y", "L_Z"});
    ALLOW_CALL(*mockObservable, getIntervalValues()).LR_RETURN(std::vector<double>(boxSize.begin(), boxSize.end()));
    ALLOW_CALL(*mockObservable, getNominalHeader()).RETURN(std::vector<std::string>{"dim"});
    ALLOW_CALL(*mockObservable, getNominalValues()).LR_RETURN(std::vector<std::string>{dimFormatter()});
    ALLOW_CALL(*mockObservable, getName()).RETURN("box dimensions");

    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Shape s1({1, 1, 1});
    Shape s2({2, 3, 3});
    Shape s3({2.5, 3, 1});
    Packing packing({3, 4, 5}, {s1, s2, s3}, std::move(pbc), mockShapeTraits.getInteraction(), 1, 1);

    ObservablesCollector collector;
    using OC = ObservablesCollector;
    collector.addObservable(std::move(mockObservable), OC::INLINE | OC::SNAPSHOT | OC::AVERAGING);
    collector.addObservable(std::make_unique<CompressibilityFactor>(), OC::SNAPSHOT | OC::AVERAGING);
    collector.addObservable(std::make_unique<NumberDensity>(), OC::INLINE | OC::SNAPSHOT);
    collector.setThermodynamicParameters(4, 2);

    SECTION("snapshots") {
        collector.addSnapshot(packing, 100, mockShapeTraits);
        packing.tryScaling(2, mockShapeTraits.getInteraction());
        collector.addSnapshot(packing, 200, mockShapeTraits);
        std::ostringstream out;

        collector.printSnapshots(out);

        CHECK(out.str() == "cycle L_X L_Y L_Z dim Z rho \n"
                           "100 3 4 5 3x4x5 10 0.050000000000000003 \n"
                           "200 6 8 10 6x8x10 80 0.0062500000000000003 \n");

        SECTION("clearing") {
            std::ostringstream out2;

            collector.clear();

            collector.printSnapshots(out2);
            CHECK(out2.str() == "cycle L_X L_Y L_Z dim Z rho \n");
        }
    }

    SECTION("average values") {
        collector.addAveragingValues(packing, mockShapeTraits);
        packing.tryScaling(2, mockShapeTraits.getInteraction());
        collector.addAveragingValues(packing, mockShapeTraits);

        SECTION("flattened") {
            auto values = collector.getFlattenedAverageValues();

            REQUIRE(values.size() == 4);
            CHECK(values[0].name == "L_X");
            CHECK(values[0].quantity.value == Approx(4.5));
            CHECK(values[1].name == "L_Y");
            CHECK(values[1].quantity.value == Approx(6));
            CHECK(values[2].name == "L_Z");
            CHECK(values[2].quantity.value == Approx(7.5));
            CHECK(values[3].name == "Z");
            CHECK(values[3].quantity.value == Approx(45));

            SECTION("clearing") {
                collector.clear();

                auto values2 = collector.getFlattenedAverageValues();

                REQUIRE(values2.size() == 4);
                auto i = GENERATE(range(0, 4));
                CHECK(values2[i].quantity.value == 0);
            }
        }

        SECTION("grouped") {
            auto values = collector.getGroupedAverageValues();

            REQUIRE(values.size() == 2);
            CHECK(values[0].groupName == "box dimensions");

            REQUIRE(values[0].observableData.size() == 3);
            CHECK(values[0].observableData[0].name == "L_X");
            CHECK(values[0].observableData[0].quantity.value == Approx(4.5));
            CHECK(values[0].observableData[1].name == "L_Y");
            CHECK(values[0].observableData[1].quantity.value == Approx(6));
            CHECK(values[0].observableData[2].name == "L_Z");
            CHECK(values[0].observableData[2].quantity.value == Approx(7.5));

            REQUIRE(values[1].observableData.size() == 1);
            CHECK(values[1].observableData[0].name == "Z");
            CHECK(values[1].observableData[0].quantity.value == Approx(45));
        }
    }

    SECTION("inline string") {
        std::string inlineString = collector.generateInlineObservablesString(packing, mockShapeTraits);

        CHECK(inlineString == "L_X: 3, L_Y: 4, L_Z: 5, dim: 3x4x5, rho: 0.05");
    }
}