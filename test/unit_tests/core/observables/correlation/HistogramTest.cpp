//
// Created by pkua on 14.09.22.
//

#include <catch2/catch.hpp>

#include "core/observables/correlation/Histogram.h"


TEST_CASE("Histogram: reduction methods") {
    // Bins: [1, 2), [2, 3]
    Histogram histogram(1, 3, 2);

    // |   |   | 5 |
    // | 2 |   | 4 |
    // [1,2)   [2,3)
    histogram.add(1.1, 2);
    histogram.add(2.1, 4);
    histogram.add(2.9, 5);
    histogram.nextSnapshot();

    // | 6 |   | 15|
    // [1,2)   [2,3)
    histogram.add(1.9, 6);
    histogram.add(2.5, 15);
    histogram.nextSnapshot();

    SECTION("sum reduction") {
        auto values = histogram.dumpValues(Histogram::ReductionMethod::SUM);
        CHECK(values == std::vector<Histogram::BinValue>{{1.5, 4}, {2.5, 12}});
    }

    SECTION("average reduction") {
        auto values = histogram.dumpValues(Histogram::ReductionMethod::AVERAGE);
        CHECK(values == std::vector<Histogram::BinValue>{{1.5, 4}, {2.5, 8}});
    }

    SECTION("clearing") {
        histogram.clear();
        histogram.add(1.5, 2);
        histogram.add(2.5, 4);
        histogram.nextSnapshot();

        auto values = histogram.dumpValues(Histogram::ReductionMethod::SUM);
        CHECK(values == std::vector<Histogram::BinValue>{{1.5, 2}, {2.5, 4}});
    }
}

TEST_CASE("Histogram: info") {
    // Bins: [1, 2), [2, 3]
    Histogram histogram(1, 3, 2);

    CHECK(histogram.size() == 2);
    CHECK(histogram.getMin() == 1);
    CHECK(histogram.getMax() == 3);
    CHECK(histogram.getBinSize() == 1);
}

TEST_CASE("Histogram: add") {
    // Bins: [1, 2), [2, 3]
    Histogram histogram(1, 3, 2);

    SECTION("tricky values") {
        histogram.add(1, 4);
        histogram.add(2, 5);
        histogram.add(3, 6);
        histogram.nextSnapshot();

        auto values = histogram.dumpValues(Histogram::ReductionMethod::SUM);
        CHECK(values == std::vector<Histogram::BinValue>{{1.5, 4}, {2.5, 11}});
    }

    SECTION("errors") {
        CHECK_THROWS(histogram.add(0.9, 5));
        CHECK_THROWS(histogram.add(3.1, 5));
    }
}