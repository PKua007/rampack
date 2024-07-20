//
// Created by Piotr Kubala on 20/07/2024.
//

#include <catch2/catch.hpp>

#include "core/lattice/param_randomizers/UniformShapeParameterRandomizer.h"
#include "utils/Quantity.h"


TEST_CASE("UniformShapeParameterRandomizer") {
    std::mt19937 mt; // NOLINT(*-msc51-cpp)

    std::vector<double> samples;
    samples.reserve(1000);

    SECTION("int") {
        UniformShapeParameterRandomizer<int> randomizer(-1, 1);

        for (std::size_t i{}; i < samples.capacity(); i++)
            samples.push_back(std::stod(randomizer.randomize("", mt)));

        std::size_t countM1 = std::count(samples.begin(), samples.end(), -1);
        std::size_t count0 = std::count(samples.begin(), samples.end(), 0);
        std::size_t countP1 = std::count(samples.begin(), samples.end(), 1);
        CHECK(countM1 == Approx(333).margin(16));       // 5% tolerance
        CHECK(count0 == Approx(333).margin(16));
        CHECK(countP1 == Approx(333).margin(16));
        CHECK(countM1 + count0 + countP1 == 1000);
    }

    SECTION("double") {
        UniformShapeParameterRandomizer<double> randomizer(-1, 1);

        for (std::size_t i{}; i < samples.capacity(); i++)
            samples.push_back(std::stod(randomizer.randomize("", mt)));

        Quantity meanQuantity;
        meanQuantity.calculateFromSamples(samples);
        double min = *std::min_element(samples.begin(), samples.end());
        double max = *std::max_element(samples.begin(), samples.end());
        double mean = meanQuantity.value;
        double stddev = meanQuantity.error * std::sqrt(samples.size());     // Recalculate sample error from mean error
        CHECK(min == Approx(-1).margin(0.1));
        CHECK(max == Approx(1).margin(0.1));
        CHECK(mean == Approx(0).margin(0.1));
        CHECK(stddev == Approx(1. / std::sqrt(3.)).margin(0.1));
    }
}