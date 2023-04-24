//
// Created by Piotr Kubala on 22/04/2023.
//

#include <sstream>
#include <iomanip>

#include <catch2/catch.hpp>
#include <ZipIterator.hpp>

#include "mocks/MockPairEnumerator.h"
#include "mocks/MockCorrelationFunction.h"

#include "core/observables/ProbabilityEvolution.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"



TEST_CASE("ProbabilityEvolution") {
    using trompeloeil::_;

    // 4 distinguishable shapes (with 4 different positions)
    std::vector<Shape> shapes;
    for (std::size_t i{}; i < 4; i++) {
        auto posDouble = static_cast<double>(i) + 0.5;
        shapes.push_back(Shape({posDouble, posDouble, posDouble}));
    }

    SphereTraits traits(0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(TriclinicBox(5), shapes, std::move(pbc), traits.getInteraction());

    // Now we basically hard code the following histogram:
    // val=[1,2]    count=1 *  count=0
    // val=[0,1]    count=2    count=1
    //            dist=[0,1]  dist=[1,2]
    // * Actual counts are divided by two, because we emit two "snapshots"
    std::vector<std::pair<std::size_t, std::size_t>> pairs = {{0, 1}, {1, 2}, {0, 3}, {0, 2}};
    std::vector<double> distances = {0.5, 0.5, 0.5, 1.5};
    std::vector<double> values = {0.5, 0.5, 1.5, 0.5};

    // We will do 2 "snapshots": 1st snapshot: pairs 0 and 1; 2nd snapshot: pairs 2 and 3
    auto enumerator = std::make_shared<MockPairEnumerator>();
    trompeloeil::sequence seq;
    REQUIRE_CALL(*enumerator, enumeratePairs(_, _, _))
        .LR_SIDE_EFFECT(
                PairConsumer &consumer = _3;
                consumer.consumePair(packing, pairs[0], distances[0], traits);
                consumer.consumePair(packing, pairs[1], distances[1], traits);
        )
        .IN_SEQUENCE(seq);
    REQUIRE_CALL(*enumerator, enumeratePairs(_, _, _))
        .LR_SIDE_EFFECT(
                PairConsumer &consumer = _3;
                consumer.consumePair(packing, pairs[2], distances[2], traits);
                consumer.consumePair(packing, pairs[3], distances[3], traits);
        )
        .IN_SEQUENCE(seq);
    ALLOW_CALL(*enumerator, getSignatureName()).RETURN("enumerator");

    auto function = std::make_shared<MockCorrelationFunction>();
    ALLOW_CALL(*function, calculate(shapes[pairs[0].first], shapes[pairs[0].second], _)).RETURN(values[0]);
    ALLOW_CALL(*function, calculate(shapes[pairs[1].first], shapes[pairs[1].second], _)).RETURN(values[1]);
    ALLOW_CALL(*function, calculate(shapes[pairs[2].first], shapes[pairs[2].second], _)).RETURN(values[2]);
    ALLOW_CALL(*function, calculate(shapes[pairs[3].first], shapes[pairs[3].second], _)).RETURN(values[3]);
    ALLOW_CALL(*function, getSignatureName()).RETURN("function");

    using Normalization = ProbabilityEvolution::Normalization;
    std::ostringstream out;
    out << std::fixed << std::setprecision(1);

    SECTION("normalization: NONE") {
        ProbabilityEvolution evolution(2, {0, 2}, 2, 2, enumerator, function, Normalization::NONE);
        evolution.addSnapshot(packing, 1, 1, traits);
        evolution.addSnapshot(packing, 1, 1, traits);

        evolution.print(out);

        CHECK(out.str() == "0.5 0.5 1.0\n0.5 1.5 0.5\n1.5 0.5 0.5\n1.5 1.5 0.0\n");
        CHECK(evolution.getSignatureName() == "prob_function_enumerator");
    }

    SECTION("normalization: PDF") {
        ProbabilityEvolution evolution(2, {0, 2}, 2, 2, enumerator, function, Normalization::PDF);
        evolution.addSnapshot(packing, 1, 1, traits);
        evolution.addSnapshot(packing, 1, 1, traits);

        evolution.print(out);

        CHECK(out.str() == "0.5 0.5 0.7\n0.5 1.5 0.3\n1.5 0.5 1.0\n1.5 1.5 0.0\n");
    }

    SECTION("normalization: UNIT") {
        ProbabilityEvolution evolution(2, {0, 2}, 2, 2, enumerator, function, Normalization::UNIT);
        evolution.addSnapshot(packing, 1, 1, traits);
        evolution.addSnapshot(packing, 1, 1, traits);

        evolution.print(out);

        CHECK(out.str() == "0.5 0.5 1.3\n0.5 1.5 0.7\n1.5 0.5 2.0\n1.5 1.5 0.0\n");
    }
}