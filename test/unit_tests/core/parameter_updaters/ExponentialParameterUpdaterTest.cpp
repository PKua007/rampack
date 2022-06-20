//
// Created by pkua on 20.06.22.
//

#include <catch2/catch.hpp>

#include "core/parameter_updaters/ExponentialParameterUpdater.h"


TEST_CASE("ExponentialParameterUpdater") {
    ExponentialParameterUpdater updater(3, std::log(2.)/100);

    CHECK(updater.getValueForCycle(0, 1000) == Approx(3));
    CHECK(updater.getValueForCycle(100, 1000) == Approx(6));
    CHECK(updater.getValueForCycle(300, 1000) == Approx(24));
    CHECK(updater.getValueForCycle(300, 2000) == Approx(24));
}