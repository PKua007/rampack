//
// Created by Piotr Kubala on 03/03/2024.
//

#include <catch2/catch.hpp>

#include "core/lattice/param_randomizers/GaussianShapeParameterRandomizer.h"
#include "RandomizerHelpers.h"


TEST_CASE("GaussianShapeParameterRandomizer") {
    SECTION("cutoff") {
        GaussianShapeParameterRandomizer randomizer(5, 1, 1.5);

        RandomizerStatistics statistics(randomizer);

        CHECK(statistics.min >= 3.5);
        CHECK(statistics.min < 3.6);
        CHECK(statistics.max <= 6.5);
        CHECK(statistics.max > 6.4);
    }

    SECTION("statistics") {
        GaussianShapeParameterRandomizer randomizer(5, 1);

        RandomizerStatistics statistics(randomizer);

        CHECK(statistics.mean == Approx(5).margin(0.1));
        CHECK(statistics.stddev == Approx(1).margin(0.1));
    }
}