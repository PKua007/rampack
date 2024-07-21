//
// Created by Piotr Kubala on 21/07/2024.
//

#include <catch2/catch.hpp>

#include "core/lattice/param_randomizers/DiscreteShapeParameterRandomizer.h"
#include "RandomizerHelpers.h"


TEST_CASE("DiscreteShapeParameterRandomizer") {
    DiscreteShapeParameterRandomizer randomizer{"a", "b", "c"};

    std::vector<std::string> samples = sample_randomizer(randomizer, 1000);

    std::size_t countM1 = std::count(samples.begin(), samples.end(), "a");
    std::size_t count0 = std::count(samples.begin(), samples.end(), "b");
    std::size_t countP1 = std::count(samples.begin(), samples.end(), "c");
    CHECK(countM1 == Approx(333).margin(16));       // 5% tolerance
    CHECK(count0 == Approx(333).margin(16));
    CHECK(countP1 == Approx(333).margin(16));
    CHECK(countM1 + count0 + countP1 == 1000);
}