//
// Created by pkua on 01.11.22.
//

#include <catch2/catch.hpp>
#include <ZipIterator.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/observables/DensityHistogram.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"
#include "core/observables/trackers/FourierTracker.h"
#include "core/observables/shape_functions/ConstantShapeFunction.h"


TEST_CASE("DensityHistogram") {
    TriclinicBox box(10);
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{4.5, 5, 1.5});
    shapes.emplace_back(Vector<3>{5.25, 5, 1.5});
    shapes.emplace_back(Vector<3>{5.75, 5, 1.5});
    shapes.emplace_back(Vector<3>{6.5, 5, 1.5});
    shapes.emplace_back(Vector<3>{4.5, 5, 7.5});
    shapes.emplace_back(Vector<3>{5.25, 5, 7.5});
    shapes.emplace_back(Vector<3>{5.75, 5, 7.5});
    shapes.emplace_back(Vector<3>{6.5, 5, 7.5});
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    SphereTraits traits(0.5);
    Packing packing(box, std::move(shapes), std::move(pbc), traits.getInteraction());
    auto densityFunction = std::make_shared<ConstantShapeFunction>();
    auto tracker = std::make_unique<FourierTracker>(std::array<std::size_t, 3>{1, 0, 0}, densityFunction);
    DensityHistogram densityHistogram({10, 0, 10}, std::move(tracker));

    for (std::size_t i{}; i < 10; i++) {
        densityHistogram.addSnapshot(packing, 1, 1, traits);
        for (std::size_t j{}; j < packing.size(); j++) {
            packing.tryTranslation(j, {0.3, 0, 0}, traits.getInteraction());
            packing.acceptTranslation();
        }
    }

    auto histogram = densityHistogram.dumpValues();
    auto zerosRemover = [](const Histogram3D::BinValue &binValue) { return binValue.value == 0; };
    histogram.erase(std::remove_if(histogram.begin(), histogram.end(), zerosRemover), histogram.end());
    std::vector<Histogram3D::BinValue> expected = {
        {{0.45, 0.5, 0.15}, 12.5}, {{0.45, 0.5, 0.75}, 12.5},
        {{0.55, 0.5, 0.15}, 25}, {{0.55, 0.5, 0.75}, 25},
        {{0.65, 0.5, 0.15}, 12.5}, {{0.65, 0.5, 0.75}, 12.5}
    };
    for (auto[expectedItem, actualItem] : Zip(expected, histogram)) {
        CHECK_THAT(actualItem.binMiddle, IsApproxEqual(expectedItem.binMiddle, 1e-12));
        CHECK(actualItem.value == Approx(expectedItem.value));
    }
}