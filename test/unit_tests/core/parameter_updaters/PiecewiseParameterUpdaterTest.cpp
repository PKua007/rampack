//
// Created by pkua on 20.06.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockParameterUpdater.h"

#include "core/parameter_updaters/PiecewiseParameterUpdater.h"

namespace {
    PiecewiseParameterUpdater::UpdaterList create_updaters(PiecewiseParameterUpdater::UpdaterList::value_type &&v1,
                                                           PiecewiseParameterUpdater::UpdaterList::value_type &&v2,
                                                           PiecewiseParameterUpdater::UpdaterList::value_type &&v3)
    {
        PiecewiseParameterUpdater::UpdaterList list;
        list.emplace_back(v1.first, std::move(v1.second));
        list.emplace_back(v2.first, std::move(v2.second));
        list.emplace_back(v3.first, std::move(v3.second));
        return list;
    }
}

TEST_CASE("PiecewiseParameterUpdater") {
    auto u1 = std::make_unique<MockParameterUpdater>();
    auto u2 = std::make_unique<MockParameterUpdater>();
    auto u3 = std::make_unique<MockParameterUpdater>();

    SECTION("0 cycles") {
        REQUIRE_CALL(*u1, getValueForCycle(0ul, 1000ul)).RETURN(5);
        PiecewiseParameterUpdater pu(create_updaters({0, std::move(u1)}, {1000, std::move(u2)}, {3000, std::move(u3)}));
        CHECK(pu.getValueForCycle(0, 4000) == 5);
    }

    SECTION("100 cycles") {
        REQUIRE_CALL(*u1, getValueForCycle(100ul, 1000ul)).RETURN(5);
        PiecewiseParameterUpdater pu(create_updaters({0, std::move(u1)}, {1000, std::move(u2)}, {3000, std::move(u3)}));
        CHECK(pu.getValueForCycle(100, 4000) == 5);
    }

    SECTION("1000 cycles") {
        REQUIRE_CALL(*u2, getValueForCycle(0ul, 2000ul)).RETURN(5);
        PiecewiseParameterUpdater pu(create_updaters({0, std::move(u1)}, {1000, std::move(u2)}, {3000, std::move(u3)}));
        CHECK(pu.getValueForCycle(1000, 4000) == 5);
    }

    SECTION("3000 cycles") {
        REQUIRE_CALL(*u3, getValueForCycle(0ul, 1000ul)).RETURN(5);
        PiecewiseParameterUpdater pu(create_updaters({0, std::move(u1)}, {1000, std::move(u2)}, {3000, std::move(u3)}));
        CHECK(pu.getValueForCycle(3000, 4000) == 5);
    }

    SECTION("3100 cycles") {
        REQUIRE_CALL(*u3, getValueForCycle(100ul, 1000ul)).RETURN(5);
        PiecewiseParameterUpdater pu(create_updaters({0, std::move(u1)}, {1000, std::move(u2)}, {3000, std::move(u3)}));
        CHECK(pu.getValueForCycle(3100, 4000) == 5);
    }

    SECTION("errors") {
        SECTION("not ordered") {
            REQUIRE_THROWS(PiecewiseParameterUpdater(create_updaters({1000, std::move(u1)}, {0, std::move(u2)},
                                                                     {3000, std::move(u3)})));
        }

        SECTION("not starting from 0") {
            REQUIRE_THROWS(PiecewiseParameterUpdater(create_updaters({1000, std::move(u1)}, {2000, std::move(u2)},
                                                                     {3000, std::move(u3)})));
        }

        SECTION("incorrect calls") {
            using trompeloeil::_;
            ALLOW_CALL(*u1, getValueForCycle(_, _)).RETURN(5);
            ALLOW_CALL(*u2, getValueForCycle(_, _)).RETURN(5);
            ALLOW_CALL(*u3, getValueForCycle(_, _)).RETURN(5);
            PiecewiseParameterUpdater parameterUpdater(create_updaters({0, std::move(u1)}, {1000, std::move(u2)},
                                                                       {3000, std::move(u3)}));

            CHECK_THROWS(parameterUpdater.getValueForCycle(5000, 4000));
            CHECK_NOTHROW(parameterUpdater.getValueForCycle(4000, 4000));
            CHECK_THROWS(parameterUpdater.getValueForCycle(1000, 2000));
        }
    }
}