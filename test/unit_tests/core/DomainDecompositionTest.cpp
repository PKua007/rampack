//
// Created by Piotr Kubala on 13/03/2021.
//

#include <catch2/catch.hpp>

#include "core/DomainDecomposition.h"
#include "core/shapes/PolysphereTraits.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("DomainDecomposition") {
    // 2 domains for dimers of radius 1, distance 1 (width: 3, range: 2, total range: 6) with origin placed at y = 17
    // packing is 12 x 21 x 6 with neighbour grid divisions 4 x 7 x 2, which gives cubic cells of size 3
    // ghost layer middles theoretically go to 0th: 19 and 1st: 6.5, to they are rounded to ranges
    // 0th: (14, 19), 1st: (5, 10), which makes the domains as 0th: (19, 5) - through pbc, 1st: (10, 14)

    double volume = 1; // volume is not important, we are laze and pick arbitrary number
    PolysphereTraits::PolysphereGeometry geometry({{{0, 0, 0}, 1}, {{1, 0, 0}, 1}},
                                                  {1, 0, 0}, {0, 1, 0}, {0, 0, 0}, volume);
    PolysphereTraits dimer(std::move(geometry));
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();

    Matrix<3, 3> id = Matrix<3, 3>::identity();
    Packing packing({12, 21, 6},
                    {{{ 7,   16, 3}, id},
                     {{ 4, 13.9, 3}, id},
                     {{ 1,    8, 3}, id},
                     {{ 8,   20, 3}, id},
                     {{ 2, 18.9, 3}, id},
                     {{ 8,   11, 3}, id},
                     {{ 8,  5.1, 3}, id},
                     {{ 1,    2, 3}, id},
                     {{ 8,  4.9, 3}, id},
                     {{ 2, 19.1, 3}, id},
                     {{10, 10.1, 3}, id},
                     {{ 4, 14.1, 3}, id},
                     {{10,  9.9, 3}, id}},
                    std::move(pbc),
                    dimer.getInteraction(),
                    dimer.getDataManager());

    DomainDecomposition domainDecomposition(packing, {1, 2, 1}, {4, 7, 2}, {6, 17, 3});

    SECTION("particles in correct domains") {
        CHECK(domainDecomposition.getParticlesInRegion({0, 0, 0}) == std::vector<std::size_t>{3, 7, 8, 9});
        CHECK(domainDecomposition.getParticlesInRegion({0, 1, 0}) == std::vector<std::size_t>{1, 5, 10});
    }

    SECTION("is vector in active region") {
        // 2 vectors: first in domain 1, second in ghost layer above
        CHECK(domainDecomposition.isVectorInActiveRegion({1, 4.9, 3}, {0, 0, 0}));
        CHECK_FALSE(domainDecomposition.isVectorInActiveRegion({1, 4.9, 3}, {0, 1, 0}));
        CHECK_FALSE(domainDecomposition.isVectorInActiveRegion({1, 5.1, 3}, {0, 0, 0}));
        CHECK_FALSE(domainDecomposition.isVectorInActiveRegion({1, 5.1, 3}, {0, 1, 0}));

        // 2 vectors: second in domain 0, first in ghost layer below
        CHECK_FALSE(domainDecomposition.isVectorInActiveRegion({2, 9.9, 3}, {0, 0, 0}));
        CHECK_FALSE(domainDecomposition.isVectorInActiveRegion({2, 9.9, 3}, {0, 1, 0}));
        CHECK_FALSE(domainDecomposition.isVectorInActiveRegion({2, 10.1, 3}, {0, 0, 0}));
        CHECK(domainDecomposition.isVectorInActiveRegion({2, 10.1, 3}, {0, 1, 0}));
    }

    SECTION("domain boundaries") {
        constexpr double inf = std::numeric_limits<double>::infinity();

        auto domain0 = domainDecomposition.getActiveDomainBounds({0, 0, 0});
        auto domain1 = domainDecomposition.getActiveDomainBounds({0, 1, 0});
        CHECK(domain0.getBoundsForCoordinate(0) == ActiveDomain::RegionBounds{-inf, inf});
        CHECK(domain0.getBoundsForCoordinate(1).beg == Approx(19./21));
        CHECK(domain0.getBoundsForCoordinate(1).end == Approx(5./21));
        CHECK(domain0.getBoundsForCoordinate(2) == ActiveDomain::RegionBounds{-inf, inf});
        CHECK(domain1.getBoundsForCoordinate(0) == ActiveDomain::RegionBounds{-inf, inf});
        CHECK(domain1.getBoundsForCoordinate(1).beg == Approx(10./21));
        CHECK(domain1.getBoundsForCoordinate(1).end == Approx(14./21));
        CHECK(domain1.getBoundsForCoordinate(2) == ActiveDomain::RegionBounds{-inf, inf});
    }
}