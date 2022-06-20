//
// Created by pkua on 20.06.22.
//

#include <catch2/catch.hpp>

#include "core/parameter_updaters/LinearParameterUpdater.h"


TEST_CASE("LinearParameterUpdater") {
    LinearParameterUpdater updater(0.01, 3.0);

    CHECK(updater.getValueForCycle(0, 1000) == Approx(3));
    CHECK(updater.getValueForCycle(100, 1000) == Approx(4));
    CHECK(updater.getValueForCycle(500, 1000) == Approx(8));
    CHECK(updater.getValueForCycle(500, 2000) == Approx(8));
}