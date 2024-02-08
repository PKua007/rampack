//
// Created by pkua on 14.09.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"
#include "matchers/VectorApproxMatcher.h"

#include "PairCollector.h"

#include "core/observables/correlation/RadialEnumerator.h"
#include "core/PeriodicBoundaryConditions.h"


TEST_CASE("RadialEnumerator") {
    using trompeloeil::_;

    TriclinicBox box(10);
    // distance pairs:
    // (0, 1) => {0, 2, 0}
    // (0, 2) => {-2, -2, -2}   through PBC
    // (1, 2) => {-2, -4, -2}   through PBC
    // (0, 0), (1, 1), (2, 2) => 0
    std::vector<Shape> shapes{Shape{{1, 1, 1}}, Shape{{1, 3, 1}}, Shape{{9, 9, 9}}};
    MockShapeTraits traits;
    ALLOW_CALL(traits, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getRangeRadius(_)).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius(_)).RETURN(1);
    ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(_1.getPosition());
    ALLOW_CALL(traits, getShapeDataSize()).RETURN(0);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(box, shapes, std::move(bc), traits.getInteraction(), traits.getDataManager());
    RadialEnumerator enumerator;

    SECTION("pair enumerating") {
        PairCollector collector;

        enumerator.enumeratePairs(packing, traits, collector);

        REQUIRE(collector.pairData.size() == 6);
        CHECK(collector.pairData.at({0, 0}) == Vector<3>{0, 0, 0});
        CHECK_THAT(collector.pairData.at({0, 1}), IsApproxEqual(Vector<3>{0, 2, 0}, 1e-12));
        CHECK_THAT(collector.pairData.at({0, 2}), IsApproxEqual(Vector<3>{-2, -2, -2}, 1e-12));
        CHECK_THAT(collector.pairData.at({1, 2}), IsApproxEqual(Vector<3>{-2, -4, -2}, 1e-12));
        CHECK(collector.pairData.at({1, 1}) == Vector<3>{0, 0, 0});
        CHECK(collector.pairData.at({2, 2}) == Vector<3>{0, 0, 0});
    }

    SECTION("number of molecules in shells") {
        auto molecules = enumerator.getExpectedNumOfMoleculesInShells(packing, {1, 2, 3});

        double numberDensity = packing.getNumberDensity();
        std::vector<double> expectedMolecules = {4./3 * M_PI * (2*2*2 - 1*1*1) * numberDensity,
                                                 4./3 * M_PI * (3*3*3 - 2*2*2) * numberDensity};
        CHECK_THAT(molecules, Catch::Matchers::Approx(expectedMolecules));
    }

    SECTION("signature name") {
        CHECK(enumerator.getSignatureName() == "r");
    }
}