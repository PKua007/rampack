//
// Created by pkua on 15.09.22.
//

#include <catch2/catch.hpp>
#include <memory>
#include <sstream>

#include "mocks/MockShapeTraits.h"
#include "mocks/MockPairEnumerator.h"
#include "mocks/MockCorrelationFunction.h"

#include "core/observables/correlation/PairAveragedCorrelation.h"
#include "core/PeriodicBoundaryConditions.h"


TEST_CASE("PairAveragedCorrelation") {
    using trompeloeil::_;

    Shape s0({1, 1, 1}), s1({2, 2, 2}), s2({3, 3, 3});
    std::vector<Shape> shapes{s0, s1, s2};
    auto binDividers = std::vector<double>{0, 2, 4};
    TriclinicBox box(10);

    MockShapeTraits traits;
    ALLOW_CALL(traits, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getRangeRadius(_)).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius(_)).RETURN(1);
    ALLOW_CALL(traits, getShapeDataSize()).RETURN(0);
    ALLOW_CALL(traits, validateShapeData(_));
    ALLOW_CALL(traits, getComparator()).RETURN(ShapeData::Comparator{});

    auto function = std::make_unique<MockCorrelationFunction>();
    ALLOW_CALL(*function, getSignatureName()).RETURN("func");
    trompeloeil::sequence seq;
    // Shapes correspond to subsequent consumePair invocations
    REQUIRE_CALL(*function, calculate(s0, s0, Vector<3>{1.5, 0, 0}, _)).LR_WITH(&_4 == &traits).IN_SEQUENCE(seq).RETURN(1);
    REQUIRE_CALL(*function, calculate(s0, s2, Vector<3>{1.8, 0, 0}, _)).LR_WITH(&_4 == &traits).IN_SEQUENCE(seq).RETURN(2);
    REQUIRE_CALL(*function, calculate(s0, s1, Vector<3>{1.5, 0, 0}, _)).LR_WITH(&_4 == &traits).IN_SEQUENCE(seq).RETURN(3);
    REQUIRE_CALL(*function, calculate(s1, s2, Vector<3>{2.5, 0, 0}, _)).LR_WITH(&_4 == &traits).IN_SEQUENCE(seq).RETURN(4);

    auto enumerator = std::make_unique<MockPairEnumerator>();
    ALLOW_CALL(*enumerator, getSignatureName()).RETURN("enum");
    // Snapshot 1 - expected bins: {1 + 2, 0}
    auto bc1 = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing1(box, shapes, std::move(bc1), traits.getInteraction(), traits.getDataManager());
    REQUIRE_CALL(*enumerator, enumeratePairs(_, _, _))
        .LR_WITH(&_1 == &packing1 && &_2 == &traits)
        .SIDE_EFFECT (
            _3.consumePair(_1, {0, 0}, {1.5, 0, 0}, _2);   // ends in BIN 1 with value 1
            _3.consumePair(_1, {0, 2}, {1.8, 0, 0}, _2);   // ends in BIN 1 with value 2
            _3.consumePair(_1, {1, 2}, {5.5, 0, 0}, _2);   // REJECTED because too big distance
        );
    // Snapshot 2 - expected bins: {3, 4}
    auto bc2 = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing2(box, shapes, std::move(bc2), traits.getInteraction(), traits.getDataManager());
    REQUIRE_CALL(*enumerator, enumeratePairs(_, _, _))
        .LR_WITH(&_1 == &packing2 && &_2 == &traits)
        .SIDE_EFFECT (
            _3.consumePair(_1, {0, 1}, {1.5, 0, 0}, _2);  // ends in BIN 1 with value 3
            _3.consumePair(_1, {1, 2}, {2.5, 0, 0}, _2);  // ends in BIN 2 with value 4
        );

    // Bin ranges: [0, 2), [2, 4]
    PairAveragedCorrelation correlation(std::move(enumerator), std::move(function), 4, 2);

    correlation.addSnapshot(packing1, 1, 1, traits);
    correlation.addSnapshot(packing2, 1, 1, traits);

    std::ostringstream out;
    correlation.print(out);
    std::string expectedOut = "1 2\n3 4\n";    // {(1 + 2 + 3) / 3, 4 / 1} = {2, 4}
    CHECK(out.str() == expectedOut);
    CHECK(correlation.getSignatureName() == "func_enum");
}