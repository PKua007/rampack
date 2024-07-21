//
// Created by Piotr Kubala on 22/07/2024.
//

#include <catch2/catch.hpp>

#include "core/lattice/param_randomizers/TriangularShapeParameterRandomizer.h"
#include "RandomizerHelpers.h"


TEST_CASE("TriangularShapeParameterRandomizer") {
    TriangularShapeParameterRandomizer randomizer(-1, 1, 2);

    RandomizerStatistics statistics(randomizer);

    CHECK(statistics.min == Approx(-1).margin(0.1));
    CHECK(statistics.max == Approx(1).margin(2));
    CHECK(statistics.mean == Approx(2./3).margin(0.1));
    CHECK(statistics.stddev == Approx(std::sqrt(3.5)/3).margin(0.1));
}