//
// Created by pkua on 14.09.22.
//

#include <catch2/catch.hpp>

#include "utils/HistogramBuilder.h"
#include "utils/OMPMacros.h"


TEST_CASE("Histogram: reduction methods") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<1> histogram(1, 3, 2);

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
        auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::SUM);
        CHECK(values == std::vector<HistogramBuilder<1>::BinValue>{{1.5, 4}, {2.5, 12}});
    }

    SECTION("average reduction") {
        auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::AVERAGE);
        CHECK(values == std::vector<HistogramBuilder<1>::BinValue>{{1.5, 4}, {2.5, 8}});
    }

    SECTION("clearing") {
        histogram.clear();
        histogram.add(1.5, 2);
        histogram.add(2.5, 4);
        histogram.nextSnapshot();

        auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::SUM);
        CHECK(values == std::vector<HistogramBuilder<1>::BinValue>{{1.5, 2}, {2.5, 4}});
    }
}

TEST_CASE("Histogram: OpenMP") {
    // The same as the above, but in parallel
    HistogramBuilder<1> histogram(1, 3, 2, 2);

    #pragma omp parallel num_threads(2) shared(histogram) default(none)
    {
        if (_OMP_THREAD_ID == 0) {
            histogram.add(1.1, 2);
        } else {
            histogram.add(2.1, 4);
            histogram.add(2.9, 5);
        }
    }
    histogram.nextSnapshot();

    #pragma omp parallel num_threads(2) shared(histogram) default(none)
    {
        if (_OMP_THREAD_ID == 0)
            histogram.add(1.9, 6);
        else
            histogram.add(2.5, 15);
    }
    histogram.nextSnapshot();

    auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::SUM);
    CHECK(values == std::vector<HistogramBuilder<1>::BinValue>{{1.5, 4}, {2.5, 12}});
}

TEST_CASE("Histogram: empty histogram") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<1> histogram(1, 3, 2);

    auto reductionMethod = GENERATE(HistogramBuilder<1>::ReductionMethod::SUM, HistogramBuilder<1>::ReductionMethod::AVERAGE);
    auto values = histogram.dumpValues(reductionMethod);
    CHECK(values == std::vector<HistogramBuilder<1>::BinValue>{{1.5, 0}, {2.5, 0}});
}

TEST_CASE("Histogram: info") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<1> histogram(1, 3, 2);

    CHECK(histogram.getNumBins() == 2);
    CHECK(histogram.getMin() == 1);
    CHECK(histogram.getMax() == 3);
    CHECK(histogram.getBinSize() == 1);
    CHECK(histogram.getBinDividers() == std::vector<double>{1, 2, 3});
}

TEST_CASE("Histogram: add") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<1> histogram(1, 3, 2);

    SECTION("tricky values") {
        histogram.add(1, 4);
        histogram.add(2, 5);
        histogram.add(3, 6);
        histogram.nextSnapshot();

        auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::SUM);
        CHECK(values == std::vector<HistogramBuilder<1>::BinValue>{{1.5, 4}, {2.5, 11}});
    }

    SECTION("errors") {
        CHECK_THROWS(histogram.add(0.9, 5));
        CHECK_THROWS(histogram.add(3.1, 5));
    }
}

TEST_CASE("Histogram: renormalization") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<1> histogram(1, 3, 2);

    histogram.add(1.5, 4);
    histogram.add(2.5, 5);
    histogram.add(2.5, 6);
    histogram.renormalizeBins({2, 3});
    histogram.nextSnapshot();

    auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::SUM);
    CHECK(values == std::vector<HistogramBuilder<1>::BinValue>{{1.5, 8}, {2.5, 33}});
}