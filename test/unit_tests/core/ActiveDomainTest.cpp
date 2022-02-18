//
// Created by Piotr Kubala on 26/03/2021.
//

#include <catch2/catch.hpp>

#include "core/ActiveDomain.h"

TEST_CASE("ActiveDomain") {
    TriclinicBox box(Matrix<3, 3>::identity());

    SECTION("normal order") {
        ActiveDomain activeDomain(box, {{{1, 2}, {3, 4}, {5, 6}}});

        CHECK(activeDomain.isInside({1.5, 3.5, 5.5}));
        CHECK_FALSE(activeDomain.isInside({0.5, 3.5, 5.5}));
        CHECK_FALSE(activeDomain.isInside({1.5, 4, 5.5}));
        CHECK_FALSE(activeDomain.isInside({1.5, 3.5, 6}));

        CHECK(activeDomain.getBoundsForCoordinate(0) == ActiveDomain::RegionBounds{1, 2});
        CHECK(activeDomain.getBoundsForCoordinate(1) == ActiveDomain::RegionBounds{3, 4});
        CHECK(activeDomain.getBoundsForCoordinate(2) == ActiveDomain::RegionBounds{5, 6});
    }

    SECTION("reversed order") {
        ActiveDomain activeDomain(box, {{{2, 1}, {3, 4}, {5, 6}}});

        CHECK_FALSE(activeDomain.isInside({1.5, 3.5, 5.5}));
        CHECK(activeDomain.isInside({0.5, 3.5, 5.5}));
        CHECK_FALSE(activeDomain.isInside({1.5, 4, 5.5}));
        CHECK_FALSE(activeDomain.isInside({1.5, 3.5, 6}));

        CHECK(activeDomain.getBoundsForCoordinate(0) == ActiveDomain::RegionBounds{2, 1});
        CHECK(activeDomain.getBoundsForCoordinate(1) == ActiveDomain::RegionBounds{3, 4});
        CHECK(activeDomain.getBoundsForCoordinate(2) == ActiveDomain::RegionBounds{5, 6});
    }
}