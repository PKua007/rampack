//
// Created by Piotr Kubala on 31/08/2023.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"
#include "PairCollector.h"

#include "core/observables/correlation/LinearEnumerator.h"
#include "core/PeriodicBoundaryConditions.h"


TEST_CASE("LinearEnumerator") {
    using trompeloeil::_;

    // Shape placement (numbers in the box are indices)
    //
    //           #######
    // 4.5      #     #
    // 3.5     #   3 #
    // 2.5    #     #
    // 1.5   #  12 #
    // 0.5  # 0   #
    // ^    #######
    // y/x > 0123456789  (+0.5)

    TriclinicBox box(std::array<Vector<3>, 3>{{{5, 0, 0}, {5, 5, 0}, {0, 0, 10}}});
    std::vector<Shape> shapes{Shape{{1.5, 0.5, 5}}, Shape{{3.5, 1.5, 5}}, Shape{{4.5, 1.5, 5}}, Shape{{6.5, 3.5, 5}}};
    MockShapeTraits traits;
    ALLOW_CALL(traits, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(_1.getPosition());
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(box, shapes, std::move(bc), traits.getInteraction());
    LinearEnumerator enumerator(LinearEnumerator::Axis::Y);

    SECTION("pair enumerating") {
        PairCollector collector;

        enumerator.enumeratePairs(packing, traits, collector);

        REQUIRE(collector.pairData.size() == 10);
        CHECK(collector.pairData.at({0, 0}) == Vector<3>{0, 0, 0});
        CHECK(collector.pairData.at({0, 1}) == Vector<3>{0, 1, 0});
        CHECK(collector.pairData.at({0, 2}) == Vector<3>{0, 1, 0});
        CHECK(collector.pairData.at({0, 3}) == Vector<3>{0, -2, 0});
        CHECK(collector.pairData.at({1, 1}) == Vector<3>{0, 0, 0});
        CHECK(collector.pairData.at({1, 2}) == Vector<3>{0, 0, 0});
        CHECK(collector.pairData.at({1, 3}) == Vector<3>{0, 2, 0});
        CHECK(collector.pairData.at({2, 2}) == Vector<3>{0, 0, 0});
        CHECK(collector.pairData.at({2, 3}) == Vector<3>{0, 2, 0});
        CHECK(collector.pairData.at({3, 3}) == Vector<3>{0, 0, 0});
    }

    SECTION("number of molecules in shells") {
        auto molecules = enumerator.getExpectedNumOfMoleculesInShells(packing, {0.5, 1, 1.5});

        std::vector<double> expectedMolecules = {0.4, 0.4};
        CHECK_THAT(molecules, Catch::Matchers::Approx(expectedMolecules));
    }

    SECTION("signature name") {
        CHECK(enumerator.getSignatureName() == "y");
    }
}