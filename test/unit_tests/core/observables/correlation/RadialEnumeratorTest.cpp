//
// Created by pkua on 14.09.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"
#include "PairCollector.h"

#include "core/observables/correlation/RadialEnumerator.h"
#include "core/PeriodicBoundaryConditions.h"


TEST_CASE("RadialEnumerator") {
    using trompeloeil::_;

    TriclinicBox box(10);
    // distance pairs:
    // (0, 1) => 2
    // (0, 2) => sqrt(12)
    // (1, 2) => sqrt(24)
    // (0, 0), (1, 1), (2, 2) => 0
    std::vector<Shape> shapes{Shape{{1, 1, 1}}, Shape{{1, 3, 1}}, Shape{{9, 9, 9}}};
    MockShapeTraits traits;
    ALLOW_CALL(traits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(_1.getPosition());
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(box, shapes, std::move(bc), traits.getInteraction());
    RadialEnumerator enumerator;

    SECTION("pair enumerating") {
        PairCollector collector;

        enumerator.enumeratePairs(packing, traits, collector);

        REQUIRE(collector.pairData.size() == 6);
        CHECK(collector.pairData.at({0, 0}) == 0);
        CHECK(collector.pairData.at({0, 1}) == Approx(2));
        CHECK(collector.pairData.at({0, 2}) == Approx(std::sqrt(12)));
        CHECK(collector.pairData.at({1, 2}) == Approx(std::sqrt(24)));
        CHECK(collector.pairData.at({1, 1}) == 0);
        CHECK(collector.pairData.at({2, 2}) == 0);
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