//
// Created by pkua on 20.06.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockParameterUpdater.h"

#include "core/parameter_updaters/ShiftedParameterUpdater.h"


TEST_CASE("ShiftedParameterUpdater") {
    SECTION("correct (non-negative cycles)") {
        auto mockUpdater = std::make_unique<MockParameterUpdater>();
        REQUIRE_CALL(*mockUpdater, getValueForCycle(100ul, 800ul)).RETURN(5);
        ShiftedParameterUpdater shiftedParameterUpdater(200, std::move(mockUpdater));

        CHECK(shiftedParameterUpdater.getValueForCycle(300, 1000) == 5);
    }

    SECTION("incorrect (negative cycles)") {
        using trompeloeil::_;
        auto mockUpdater = std::make_unique<MockParameterUpdater>();
        ALLOW_CALL(*mockUpdater, getValueForCycle(_, _)).RETURN(5);
        ShiftedParameterUpdater shiftedParameterUpdater(1000, std::move(mockUpdater));

        CHECK_THROWS(shiftedParameterUpdater.getValueForCycle(100, 2000));
        CHECK_THROWS(shiftedParameterUpdater.getValueForCycle(2000, 100));
    }
}