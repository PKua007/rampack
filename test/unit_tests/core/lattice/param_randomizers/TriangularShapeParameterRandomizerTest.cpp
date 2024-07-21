//
// Created by Piotr Kubala on 22/07/2024.
//

#include <catch2/catch.hpp>

#include "core/lattice/param_randomizers/TriangularShapeParameterRandomizer.h"
#include "utils/Quantity.h"


TEST_CASE("TriangularShapeParameterRandomizer") {
    std::mt19937 mt; // NOLINT(*-msc51-cpp)

    std::vector<double> samples;
    samples.reserve(1000);

    TriangularShapeParameterRandomizer randomizer(-1, 1, 2);

    for (std::size_t i{}; i < samples.capacity(); i++)
        samples.push_back(std::stod(randomizer.randomize("", mt)));

    Quantity meanQuantity;
    meanQuantity.calculateFromSamples(samples);
    double min = *std::min_element(samples.begin(), samples.end());
    double max = *std::max_element(samples.begin(), samples.end());
    double mean = meanQuantity.value;
    double stddev = meanQuantity.error * std::sqrt(samples.size());     // Recalculate sample error from mean error
    CHECK(min == Approx(-1).margin(0.1));
    CHECK(max == Approx(1).margin(2));
    CHECK(mean == Approx(2./3).margin(0.1));
    CHECK(stddev == Approx(std::sqrt(3.5)/3).margin(0.1));
}