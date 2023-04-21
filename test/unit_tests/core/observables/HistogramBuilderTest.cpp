//
// Created by pkua on 14.09.22.
//

#include "catch2/catch.hpp"

#include "core/observables/HistogramBuilder.h"
#include "utils/OMPMacros.h"


TEST_CASE("Histogram 1D: reduction methods") {
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
        CHECK(values == std::vector<BinValue<1>>{{1.5, 4}, {2.5, 12}});
    }

    SECTION("average reduction") {
        auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::AVERAGE);
        CHECK(values == std::vector<BinValue<1>>{{1.5, 4}, {2.5, 8}});
    }

    SECTION("clearing") {
        histogram.clear();
        histogram.add(1.5, 2);
        histogram.add(2.5, 4);
        histogram.nextSnapshot();

        auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::SUM);
        CHECK(values == std::vector<BinValue<1>>{{1.5, 2}, {2.5, 4}});
    }
}

#ifdef _OPENMP
TEST_CASE("Histogram 1D: OpenMP") {
    // The same as the above, but in parallel
    HistogramBuilder<1> histogram(1, 3, 2, 2);

    #pragma omp parallel num_threads(2) shared(histogram) default(none)
    {
        if (OMP_THREAD_ID == 0) {
            histogram.add(1.1, 2);
        } else {
            histogram.add(2.1, 4);
            histogram.add(2.9, 5);
        }
    }
    histogram.nextSnapshot();

    #pragma omp parallel num_threads(2) shared(histogram) default(none)
    {
        if (OMP_THREAD_ID == 0)
            histogram.add(1.9, 6);
        else
            histogram.add(2.5, 15);
    }
    histogram.nextSnapshot();

    auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::SUM);
    CHECK(values == std::vector<BinValue<1>>{{1.5, 4}, {2.5, 12}});
}
#endif // _OPENMP

TEST_CASE("Histogram 1D: empty histogram") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<1> histogram(1, 3, 2);

    auto reductionMethod = GENERATE(HistogramBuilder<1>::ReductionMethod::SUM, HistogramBuilder<1>::ReductionMethod::AVERAGE);
    auto values = histogram.dumpValues(reductionMethod);
    CHECK(values == std::vector<BinValue<1>>{{1.5, 0}, {2.5, 0}});
}

TEST_CASE("Histogram 2D: info") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<2> histogram({1, 2}, {3, 4}, {2, 4});

    CHECK(histogram.getNumBins(0) == 2);
    CHECK(histogram.getMin(0) == 1);
    CHECK(histogram.getMax(0) == 3);
    CHECK(histogram.getBinSize(0) == 1);
    CHECK(histogram.getBinDividers(0) == std::vector<double>{1, 2, 3});

    CHECK(histogram.getNumBins(1) == 4);
    CHECK(histogram.getMin(1) == 2);
    CHECK(histogram.getMax(1) == 4);
    CHECK(histogram.getBinSize(1) == 0.5);
    CHECK(histogram.getBinDividers(1) == std::vector<double>{2, 2.5, 3, 3.5, 4});
}

TEST_CASE("Histogram 1D: add") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<1> histogram(1, 3, 2);

    SECTION("tricky values") {
        histogram.add(1, 4);
        histogram.add(2, 5);
        histogram.add(3, 6);
        histogram.nextSnapshot();

        auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::SUM);
        CHECK(values == std::vector<BinValue<1>>{{1.5, 4}, {2.5, 11}});
    }

    SECTION("errors") {
        CHECK_THROWS(histogram.add(0.9, 5));
        CHECK_THROWS(histogram.add(3.1, 5));
    }
}

TEST_CASE("Histogram 2D: add") {
    // Bins: { [1, 2), [2, 3] }  X  { [1, 2), [2, 3), [3, 4] }
    HistogramBuilder<2> histogram({1, 1}, {3, 4}, {2, 3});
    histogram.add({1.2, 1.3}, 1);
    histogram.add({1.5, 1.6}, 2);
    histogram.add({2.5, 1.6}, 5);
    histogram.add({2.6, 3.1}, 7);
    histogram.nextSnapshot();

    auto values = histogram.dumpValues(HistogramBuilder<2>::ReductionMethod::SUM);
    auto expected = std::vector<BinValue<2>>{
        {{1.5, 1.5}, 3}, {{1.5, 2.5}, 0}, {{1.5, 3.5}, 0},
        {{2.5, 1.5}, 5}, {{2.5, 2.5}, 0}, {{2.5, 3.5}, 7}
    };
    CHECK_THAT(values, Catch::UnorderedEquals(expected));
}

TEST_CASE("Histogram 1D: renormalization") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<1> histogram(1, 3, 2);

    histogram.add(1.5, 4);
    histogram.add(2.5, 5);
    histogram.add(2.5, 6);
    histogram.renormalizeBins({2, 3});
    histogram.nextSnapshot();

    auto values = histogram.dumpValues(HistogramBuilder<1>::ReductionMethod::SUM);
    CHECK(values == std::vector<BinValue<1>>{{1.5, 8}, {2.5, 33}});
}