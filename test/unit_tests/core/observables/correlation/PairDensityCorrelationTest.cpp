//
// Created by pkua on 14.09.22.
//

#include <catch2/catch.hpp>
#include <memory>
#include <sstream>

#include "mocks/MockShapeTraits.h"
#include "mocks/MockPairEnumerator.h"

#include "core/observables/correlation/PairDensityCorrelation.h"
#include "core/PeriodicBoundaryConditions.h"


TEST_CASE("PairDensityCorrelation") {
    using trompeloeil::_;
    auto enumerator = std::make_unique<MockPairEnumerator>();
    ALLOW_CALL(*enumerator, getSignatureName()).RETURN("aaa");

    MockShapeTraits traits;
    ALLOW_CALL(traits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getNamedPoint(_, _)).LR_RETURN(traits.ShapeGeometry::getNamedPoint(_1, _2));

    TriclinicBox box(10);
    auto binDividers = std::vector<double>{0, 2, 4};
    std::vector<Shape> shapes(4);   // 4 shapes (with whatever positions and orientations)

    // Formula for each bin value: (num points) / (expected molecules) / (N/2)
    // Snapshot 1 - expected bins: {0/1/2, 2/2/2} = {0, 0.5}
    auto bc1 = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing1(box, shapes, std::move(bc1), traits.getInteraction());
    REQUIRE_CALL(*enumerator, enumeratePairs(_, _, _))
        .LR_WITH(&_1 == &packing1 && &_2 == &traits)
        .SIDE_EFFECT (
            _3.consumePair(_1, {0, 0}, 1.5);   // REJECTED because 0 = 0
            _3.consumePair(_1, {0, 1}, 2.5);   // ends in BIN 2
            _3.consumePair(_1, {0, 2}, 3.5);   // ends in BIN 2
            _3.consumePair(_1, {1, 2}, 5.5);   // REJECTED because too big distance
        );
    REQUIRE_CALL(*enumerator, getExpectedNumOfMoleculesInShells(_, binDividers))
        .LR_WITH(&_1 == &packing1)
        .RETURN(std::vector<double>{1, 2});

    // Snapshot 2 - expected bins: {1/0.5/2, 1/0.25/2} = {1, 2}
    auto bc2 = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing2(box, shapes, std::move(bc2), traits.getInteraction());
    REQUIRE_CALL(*enumerator, enumeratePairs(_, _, _))
        .LR_WITH(&_1 == &packing2 && &_2 == &traits)
        .SIDE_EFFECT (
                _3.consumePair(_1, {0, 1}, 1.5);  // ends in BIN 1
                _3.consumePair(_1, {1, 2}, 2.5);  // ends in BIN 2
        );
    REQUIRE_CALL(*enumerator, getExpectedNumOfMoleculesInShells(_, binDividers))
        .LR_WITH(&_1 == &packing2)
        .RETURN(std::vector<double>{0.5, 0.25});

    // Bin ranges: [0, 2), [2, 4]
    PairDensityCorrelation correlation(std::move(enumerator), 4, 2);

    correlation.addSnapshot(packing1, 1, 1, traits);
    correlation.addSnapshot(packing2, 1, 1, traits);

    std::ostringstream out;
    correlation.print(out);
    std::string expectedOut = "1 0.5\n3 1.25\n";
    CHECK(out.str() == expectedOut);
    CHECK(correlation.getSignatureName() == "rho_aaa");
}