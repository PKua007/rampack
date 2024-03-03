//
// Created by Piotr Kubala on 03/03/2024.
//

#include <catch2/catch.hpp>
#include <limits>

#include "core/lattice/GaussianShapeParameterRandomizer.h"
#include "utils/Quantity.h"


TEST_CASE("GaussianShapeParameterRandomizer") {
    constexpr double INF = std::numeric_limits<double>::infinity();
    std::mt19937 mt; // NOLINT(*-msc51-cpp)

    SECTION("cutoff") {
        GaussianShapeParameterRandomizer randomizer(5, 1, 1.5);
        double min = INF;
        double max = -INF;

        for (std::size_t i{}; i < 1000; i++) {
            double val = std::stod(randomizer.randomize("", mt));
            min = std::min(min, val);
            max = std::max(max, val);
        }

        CHECK(min >= 3.5);
        CHECK(min < 3.6);
        CHECK(max <= 6.5);
        CHECK(max > 6.4);
    }

    SECTION("statistics") {
        GaussianShapeParameterRandomizer randomizer(5, 1);
        std::vector<double> samples;
        samples.reserve(1000);

        for (std::size_t i{}; i < samples.capacity(); i++)
            samples.push_back(std::stod(randomizer.randomize("", mt)));

        Quantity mean;
        mean.calculateFromSamples(samples);
        mean.error *= std::sqrt(samples.size());    // Recalculate sample error from mean error
        CHECK(mean.value == Approx(5).margin(0.1));
        CHECK(mean.error == Approx(1).margin(0.1));
    }
}