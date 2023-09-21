//
// Created by pkua on 14.09.22.
//

#include <catch2/catch.hpp>
#include <valarray>

#include "core/observables/HistogramBuilder.h"
#include "utils/OMPMacros.h"


namespace {
    using ValarrayBinValue = Histogram<1, std::valarray<double>>::BinValue;
    using ValarrayBinValueCount = Histogram<1, CountingAccumulator<std::valarray<double>>>::BinValue;

    void compare_valarray_bin_value(const ValarrayBinValue &bv1, const ValarrayBinValue &bv2) {
        CHECK(bv1.binMiddle == bv2.binMiddle);
        REQUIRE(bv1.value.size() == bv2.value.size());
        for (std::size_t i{}; i < bv1.value.size(); i++)
            CHECK(bv1.value[i] == bv2.value[i]);
    }

    void compare_valarray_bin_value_count(const ValarrayBinValueCount &bvc1, const ValarrayBinValueCount &bvc2) {
        CHECK(bvc1.binMiddle == bvc2.binMiddle);
        CHECK(bvc1.value.numPoints == bvc2.value.numPoints);
        REQUIRE(bvc1.value.value.size() == bvc2.value.value.size());
        for (std::size_t i{}; i < bvc1.value.value.size(); i++)
            CHECK(bvc1.value.value[i] == bvc2.value.value[i]);
    }
}


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
        auto values = histogram.dumpValues(ReductionMethod::SUM);
        auto valueCounts = histogram.dumpValuesWithCount(ReductionMethod::SUM);

        using BinValue = decltype(values)::value_type;
        using BinValueCount = decltype(valueCounts)::value_type;
        CHECK(values == std::vector<BinValue>{{1.5, 4}, {2.5, 12}});
        CHECK(valueCounts == std::vector<BinValueCount>{{1.5, {4, 2}}, {2.5, {12, 3}}});
    }

    SECTION("average reduction") {
        auto values = histogram.dumpValues(ReductionMethod::AVERAGE);
        auto valueCounts = histogram.dumpValuesWithCount(ReductionMethod::AVERAGE);

        using BinValue = decltype(values)::value_type;
        using BinValueCount = decltype(valueCounts)::value_type;
        CHECK(values == std::vector<BinValue>{{1.5, 4}, {2.5, 8}});
        CHECK(valueCounts == std::vector<BinValueCount>{{1.5, {4, 2}}, {2.5, {8, 3}}});
    }

    SECTION("clearing") {
        histogram.clear();
        histogram.add(1.5, 2);
        histogram.add(2.5, 4);
        histogram.nextSnapshot();

        auto values = histogram.dumpValues(ReductionMethod::SUM);
        auto valueCounts = histogram.dumpValuesWithCount(ReductionMethod::SUM);

        using BinValue = decltype(values)::value_type;
        using BinValueCount = decltype(valueCounts)::value_type;
        CHECK(values == std::vector<BinValue>{{1.5, 2}, {2.5, 4}});
        CHECK(valueCounts == std::vector<BinValueCount>{{1.5, {2, 1}}, {2.5, {4, 1}}});
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

    auto values = histogram.dumpValues(ReductionMethod::SUM);
    auto valueCounts = histogram.dumpValuesWithCount(ReductionMethod::SUM);

    using BinValue = decltype(values)::value_type;
    using BinValueCount = decltype(valueCounts)::value_type;
    CHECK(values == std::vector<BinValue>{{1.5, 4}, {2.5, 12}});
    CHECK(valueCounts == std::vector<BinValueCount>{{1.5, {4, 2}}, {2.5, {12, 3}}});
}
#endif // _OPENMP

TEST_CASE("Histogram 1D: empty histogram") {
    // Bins: [1, 2), [2, 3]
    HistogramBuilder<1> histogram(1, 3, 2);

    auto reductionMethod = GENERATE(ReductionMethod::SUM, ReductionMethod::AVERAGE);
    auto values = histogram.dumpValues(reductionMethod);
    auto valueCounts = histogram.dumpValuesWithCount(reductionMethod);

    using BinValue = decltype(values)::value_type;
    using BinValueCount = decltype(valueCounts)::value_type;
    CHECK(values == std::vector<BinValue>{{1.5, 0}, {2.5, 0}});
    CHECK(valueCounts == std::vector<BinValueCount>{{1.5, {0, 0}}, {2.5, {0, 0}}});
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

        auto values = histogram.dumpValues(ReductionMethod::SUM);
        auto valueCounts = histogram.dumpValuesWithCount(ReductionMethod::SUM);

        using BinValue = decltype(values)::value_type;
        using BinValueCount = decltype(valueCounts)::value_type;
        CHECK(values == std::vector<BinValue>{{1.5, 4}, {2.5, 11}});
        CHECK(valueCounts == std::vector<BinValueCount>{{1.5, {4, 1}}, {2.5, {11, 2}}});
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

    auto values = histogram.dumpValues(ReductionMethod::SUM);
    auto expected = std::vector<Histogram2D::BinValue>{
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

    auto values = histogram.dumpValues(ReductionMethod::SUM);
    auto valueCounts = histogram.dumpValuesWithCount(ReductionMethod::SUM);

    using BinValue = decltype(values)::value_type;
    using BinValueCount = decltype(valueCounts)::value_type;
    CHECK(values == std::vector<BinValue>{{1.5, 8}, {2.5, 33}});
    CHECK(valueCounts == std::vector<BinValueCount>{{1.5, {8, 1}}, {2.5, {33, 2}}});
}

TEST_CASE("Histogram: non-trivial initial value") {
    HistogramBuilder<1, std::valarray<double>> histogram(0, 1, 2, 1, std::valarray<double>(0.0, 2));

    SECTION("reduction") {
        histogram.add(0.3, {1, 2});
        histogram.add(0.7, {-4, 7});
        histogram.nextSnapshot();
        histogram.add(0.3, {5, 12});
        histogram.add(0.7, {-6, -1});
        histogram.nextSnapshot();

        auto values = histogram.dumpValues(ReductionMethod::AVERAGE);
        auto valueCounts = histogram.dumpValuesWithCount(ReductionMethod::AVERAGE);

        compare_valarray_bin_value(values[0], ValarrayBinValue(0.25, {3, 7}));
        compare_valarray_bin_value(values[1], ValarrayBinValue(0.75, {-5, 3}));
        compare_valarray_bin_value_count(valueCounts[0], ValarrayBinValueCount(0.25, {{3, 7}, 2}));
        compare_valarray_bin_value_count(valueCounts[1], ValarrayBinValueCount(0.75, {{-5, 3}, 2}));

        SECTION("clearing") {
            histogram.clear();

            values = histogram.dumpValues(ReductionMethod::AVERAGE);
            valueCounts = histogram.dumpValuesWithCount(ReductionMethod::AVERAGE);

            compare_valarray_bin_value(values[0], ValarrayBinValue(0.25, {0, 0}));
            compare_valarray_bin_value(values[1], ValarrayBinValue(0.75, {0, 0}));
            compare_valarray_bin_value_count(valueCounts[0], ValarrayBinValueCount(0.25, {{0, 0}, 0}));
            compare_valarray_bin_value_count(valueCounts[1], ValarrayBinValueCount(0.75, {{0, 0}, 0}));
        }
    }
}