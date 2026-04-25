//
// Created by Codex on 16/03/2026.
//

#include <catch2/catch.hpp>

#include "core/interactions/CentrePairDataMap.h"
#include "utils/Exceptions.h"

namespace {
    struct PairData {
        // Don't zero-initialize on purpose to validate that CentrePairDataMap handles that part correctly
        double epsilon;
        double sigma;
    };
}

TEST_CASE("CentrePairDataMap") {
    SECTION("default constructor") {
        const CentrePairDataMap<PairData> pairDataMap;

        CHECK(pairDataMap.getNumCentreTypes() == 0);
    }

    SECTION("default initialization") {
        const CentrePairDataMap<PairData> pairDataMap(3);

        CHECK(pairDataMap.getNumCentreTypes() == 3);

        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                CHECK(pairDataMap.getPairData(i, j).epsilon == 0);
                CHECK(pairDataMap.getPairData(i, j).sigma == 0);
            }
        }
    }

    SECTION("setting pair data symmetrically") {
        CentrePairDataMap<PairData> pairDataMap(3);
        constexpr PairData data{1.5, 2.5};

        pairDataMap.setPairData(0, 2, data);

        CHECK(pairDataMap.getPairData(0, 2).epsilon == Approx(1.5));
        CHECK(pairDataMap.getPairData(0, 2).sigma == Approx(2.5));
        CHECK(pairDataMap.getPairData(2, 0).epsilon == Approx(1.5));
        CHECK(pairDataMap.getPairData(2, 0).sigma == Approx(2.5));
        CHECK(pairDataMap.getPairData(1, 1).epsilon == 0);
        CHECK(pairDataMap.getPairData(1, 1).sigma == 0);
    }

    SECTION("invalid indices in setter") {
        CentrePairDataMap<PairData> pairDataMap(3);

        CHECK_THROWS_AS(pairDataMap.setPairData(3, 0, {1, 2}), PreconditionException);
        CHECK_THROWS_AS(pairDataMap.setPairData(0, 3, {1, 2}), PreconditionException);
    }
}
