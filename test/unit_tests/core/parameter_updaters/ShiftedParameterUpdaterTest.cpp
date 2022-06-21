//
// Created by pkua on 20.06.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockParameterUpdater.h"

#include "core/parameter_updaters/ShiftedParameterUpdater.h"

using trompeloeil::_;


TEST_CASE("ShiftedParameterUpdater: positive shift") {
    SECTION("correct (non-negative cycles)") {
        auto mockUpdater = std::make_unique<MockParameterUpdater>();
        REQUIRE_CALL(*mockUpdater, getValueForCycle(100ul, 800ul)).RETURN(5);
        ShiftedParameterUpdater shiftedParameterUpdater(200, std::move(mockUpdater));

        CHECK(shiftedParameterUpdater.getValueForCycle(300, 1000) == 5);
    }

    SECTION("incorrect (negative cycles)") {
        auto mockUpdater = std::make_unique<MockParameterUpdater>();
        ALLOW_CALL(*mockUpdater, getValueForCycle(_, _)).RETURN(5);
        ShiftedParameterUpdater shiftedParameterUpdater(1000, std::move(mockUpdater));

        CHECK_THROWS(shiftedParameterUpdater.getValueForCycle(100, 2000));
        CHECK_THROWS(shiftedParameterUpdater.getValueForCycle(2000, 100));
    }
}

TEST_CASE("ShiftedParameterUpdater: negative shift") {
    SECTION("simple") {
        auto mockUpdater = std::make_unique<MockParameterUpdater>();
        REQUIRE_CALL(*mockUpdater, getValueForCycle(500ul, 1200ul)).RETURN(5);
        ShiftedParameterUpdater shiftedParameterUpdater(-200, std::move(mockUpdater));

        CHECK(shiftedParameterUpdater.getValueForCycle(300, 1000) == 5);
    }

    SECTION("totalCycles passing std::size_t max") {
        using trompeloeil::_;
        auto mockUpdater = std::make_unique<MockParameterUpdater>();
        REQUIRE_CALL(*mockUpdater, getValueForCycle(1300ul, std::numeric_limits<std::size_t>::max())).RETURN(5);
        ShiftedParameterUpdater shiftedParameterUpdater(-1000, std::move(mockUpdater));

        CHECK(shiftedParameterUpdater.getValueForCycle(300, std::numeric_limits<std::size_t>::max() - 999) == 5);
    }

    SECTION("incorrect: too large current cycle") {
        using trompeloeil::_;
        auto mockUpdater = std::make_unique<MockParameterUpdater>();
        ALLOW_CALL(*mockUpdater, getValueForCycle(_, _)).RETURN(5);
        ShiftedParameterUpdater shiftedParameterUpdater(-1000, std::move(mockUpdater));

        CHECK_THROWS(shiftedParameterUpdater.getValueForCycle(std::numeric_limits<std::size_t>::max() - 999,
                                                              std::numeric_limits<std::size_t>::max()) == 5);
    }
}