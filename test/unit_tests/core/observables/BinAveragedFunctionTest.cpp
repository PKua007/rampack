//
// Created by Piotr Kubala on 15/09/2023.
//

#include <catch2/catch.hpp>
#include <ZipIterator.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "mocks/MockShapeFunction.h"

#include "core/observables/BinAveragedFunction.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"
#include "core/observables/trackers/FourierTracker.h"
#include "core/observables/shape_functions/ConstantShapeFunction.h"


TEST_CASE("BinAveragedFunction") {
    TriclinicBox box(10);

    // Bins values
    // 0.45:  1
    // 0.55:  (2 + 4)/2 = 3
    // 0.65:  5
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{4.5, 5, 5});
    shapes.emplace_back(Vector<3>{5.5, 5, 1.5});
    shapes.emplace_back(Vector<3>{5.5, 5, 7.5});
    shapes.emplace_back(Vector<3>{6.5, 5, 5});

    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    SphereTraits traits(0.5);
    Packing packing(box, std::move(shapes), std::move(pbc), traits.getInteraction());

    using trompeloeil::_;
    auto mockShapeFunction = std::make_unique<MockShapeFunction>();
    double functionValue{};
    ALLOW_CALL(*mockShapeFunction, calculate(std::ref(packing[0]), _)).LR_SIDE_EFFECT(functionValue = 1);
    ALLOW_CALL(*mockShapeFunction, calculate(std::ref(packing[1]), _)).LR_SIDE_EFFECT(functionValue = 2);
    ALLOW_CALL(*mockShapeFunction, calculate(std::ref(packing[2]), _)).LR_SIDE_EFFECT(functionValue = 4);
    ALLOW_CALL(*mockShapeFunction, calculate(std::ref(packing[3]), _)).LR_SIDE_EFFECT(functionValue = 5);
    ALLOW_CALL(*mockShapeFunction, getValues()).LR_RETURN(std::vector<double>{functionValue});
    ALLOW_CALL(*mockShapeFunction, getPrimaryName()).RETURN("func");

    auto densityFunction = std::make_shared<ConstantShapeFunction>();
    auto tracker = std::make_unique<FourierTracker>(std::array<std::size_t, 3>{1, 0, 0}, densityFunction);
    BinAveragedFunction binAveragedFunction({10, 0, 0}, std::move(mockShapeFunction), std::move(tracker));

    SECTION("the histogram") {
        // We do Fourier tracking on the density - in each snapshot we move a bit in the x direction
        for (std::size_t i{}; i < 10; i++) {
            binAveragedFunction.addSnapshot(packing, 1, 1, traits);
            for (std::size_t j{}; j < packing.size(); j++) {
                packing.tryTranslation(j, {0.3, 0, 0}, traits.getInteraction());
                packing.acceptTranslation();
            }
        }

        auto histogram = binAveragedFunction.dumpValues();
        using BinValue = decltype(histogram)::value_type;
        auto nanRemover = [](const BinValue &binValue) { return std::isnan(binValue.value[0]); };
        histogram.erase(std::remove_if(histogram.begin(), histogram.end(), nanRemover), histogram.end());
        std::vector<BinValue> expected = {
            {{0.45, 0.5, 0.5}, {1}, 10},
            {{0.55, 0.5, 0.5}, {3}, 20},
            {{0.65, 0.5, 0.5}, {5}, 10}
        };
        for (auto [expectedItem, actualItem]: Zip(expected, histogram)) {
            CHECK_THAT(actualItem.binMiddle, IsApproxEqual(expectedItem.binMiddle, 1e-12));
            CHECK(actualItem.value[0] == Approx(expectedItem.value[0]));
            CHECK(actualItem.count == Approx(expectedItem.count));
        }
    }

    SECTION("signature name") {
        CHECK(binAveragedFunction.getSignatureName() == "func_xyz");
    }
}