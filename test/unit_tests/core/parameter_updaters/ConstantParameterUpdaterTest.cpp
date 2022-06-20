//
// Created by pkua on 20.06.22.
//

#include <catch2/catch.hpp>

#include "core/parameter_updaters/ConstantParameterUpdater.h"


TEST_CASE("ConstantParameterUpdater") {
    ConstantParameterUpdater updater(5);

    CHECK(updater.getValueForCycle(0, 1000) == 5);
    CHECK(updater.getValueForCycle(500, 1000) == 5);
    CHECK(updater.getValueForCycle(1000, 2000) == 5);
}