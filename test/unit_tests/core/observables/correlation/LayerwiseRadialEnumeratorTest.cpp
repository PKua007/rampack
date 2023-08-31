//
// Created by pkua on 14.09.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"
#include "PairCollector.h"

#include "core/observables/correlation/LayerwiseRadialEnumerator.h"
#include "core/PeriodicBoundaryConditions.h"


TEST_CASE("LayerwiseRadialEnumerator") {
    using trompeloeil::_;

    TriclinicBox box(std::array<Vector<3>, 3>{Vector<3>{12, 0, 0}, {4, 8, 0}, {0, 0, 10}});
    // distance vectors without those lying in different layers:
    // (0, 1) => (3, 0)
    // (2, 3) => (5, 0)
    // (2, 4) => (-2, 2)
    // (3, 4) => (3, 2)
    // (i, i) => 0
    std::vector<Shape> shapes{Shape{{2, 1, 5}}, Shape{{5, 1, 5}}, Shape{{6, 4, 5}}, Shape{{13, 4, 5}},
                              Shape{{4, 6, 5}}};
    MockShapeTraits traits;
    ALLOW_CALL(traits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(_1.getPosition());
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(box, shapes, std::move(bc), traits.getInteraction());
    // For this Miller indices: 2 skew layers joining via pbc.
    // Vector tangent to a layer (joining its ends on "y" box sides): {14, 4, 0}
    LayerwiseRadialEnumerator enumerator({1, -2, 0});

    SECTION("pair enumerating") {
        PairCollector collector;
        auto tangent = Vector<2>{7, 2}.normalized();

        enumerator.enumeratePairs(packing, traits, collector);

        REQUIRE(collector.pairData.size() == 9);
        CHECK(collector.pairData.at({0, 0}) == 0);
        CHECK(collector.pairData.at({0, 1}) == Approx(std::abs(tangent * Vector<2>{3, 0})));
        CHECK(collector.pairData.at({1, 1}) == 0);
        CHECK(collector.pairData.at({2, 2}) == 0);
        CHECK(collector.pairData.at({2, 3}) == Approx(std::abs(tangent * Vector<2>{5, 0})));
        CHECK(collector.pairData.at({2, 4}) == Approx(std::abs(tangent * Vector<2>{-2, 2})));
        CHECK(collector.pairData.at({3, 3}) == 0);
        CHECK(collector.pairData.at({3, 4}) == Approx(std::abs(tangent * Vector<2>{3, 2})));
        CHECK(collector.pairData.at({4, 4}) == 0);
    }

    SECTION("number of molecules in shells") {
        auto molecules = enumerator.getExpectedNumOfMoleculesInShells(packing, {1, 2, 3});

        double numMolecules = 5;
        //Vector joining layer ends on box sides: {14, 4, 0}, length 2*sqrt(53). There are 2 such layers, and
        // z side length is 10. This gives altogether:
        double totalLayerArea = 2 * (2 * std::sqrt(53)) * 10;
        std::vector<double> expectedMolecules = {numMolecules * M_PI * (2*2 - 1*1) / totalLayerArea,
                                                 numMolecules * M_PI * (3*3 - 2*2) / totalLayerArea};
        CHECK_THAT(molecules, Catch::Matchers::Approx(expectedMolecules));
    }

    SECTION("signature name") {
        CHECK(enumerator.getSignatureName() == "lr");
    }
}