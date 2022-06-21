//
// Created by pkua on 20.06.22.
//

#include <catch2/catch.hpp>

#include "core/dynamic_parameters/ExponentialDynamicParameter.h"


TEST_CASE("ExponentialParameterUpdater") {
    ExponentialDynamicParameter updater(3, std::log(2.) / 100);

    CHECK(updater.getValueForCycle(0, 1000) == Approx(3));
    CHECK(updater.getValueForCycle(100, 1000) == Approx(6));
    CHECK(updater.getValueForCycle(300, 1000) == Approx(24));
    CHECK(updater.getValueForCycle(300, 2000) == Approx(24));
}