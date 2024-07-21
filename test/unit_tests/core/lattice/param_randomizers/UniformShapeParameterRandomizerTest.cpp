//
// Created by Piotr Kubala on 20/07/2024.
//

#include <catch2/catch.hpp>

#include "core/lattice/param_randomizers/UniformShapeParameterRandomizer.h"
#include "RandomizerHelpers.h"


TEST_CASE("UniformShapeParameterRandomizer") {
    SECTION("int") {
        UniformShapeParameterRandomizer<int> randomizer(-1, 1);

        auto samples = sample_randomizer(randomizer, 1000);

        std::size_t countM1 = std::count(samples.begin(), samples.end(), "-1");
        std::size_t count0 = std::count(samples.begin(), samples.end(), "0");
        std::size_t countP1 = std::count(samples.begin(), samples.end(), "1");
        CHECK(countM1 == Approx(333).margin(16));       // 5% tolerance
        CHECK(count0 == Approx(333).margin(16));
        CHECK(countP1 == Approx(333).margin(16));
        CHECK(countM1 + count0 + countP1 == 1000);
    }

    SECTION("double") {
        UniformShapeParameterRandomizer<double> randomizer(-1, 1);

        RandomizerStatistics statistics(randomizer);

        CHECK(statistics.min == Approx(-1).margin(0.1));
        CHECK(statistics.max == Approx(1).margin(0.1));
        CHECK(statistics.mean == Approx(0).margin(0.1));
        CHECK(statistics.stddev == Approx(1. / std::sqrt(3.)).margin(0.1));
    }
}