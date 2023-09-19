//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_HISTOGRAMBUILDER_H
#define RAMPACK_HISTOGRAMBUILDER_H

#include <ostream>
#include <vector>
#include <array>
#include <tuple>

#include "geometry/Vector.h"
#include "Histogram.h"


/**
 * @brief Reduction method for snapshot averaging.
 */
enum class ReductionMethod {
    /**
     * @brief For each snapshot all bin values are summed. Then, those sums are averaged over snapshots.
     * @details It is suitable if one wants to obtain a standard histogram by counting points in the bins and then
     * average obtained histograms over many system snapshots.
     */
    SUM,
    /**
     * @brief All single values from all snapshots are treated as one set and the average of them is calculated.
     * @details It is suitable if one wants to calculate averages in bins for many points from many snapshots.
     */
    AVERAGE
};


/**
 * @brief Class facilitating multithreaded building a histogram, where each bin accumulates arbitrary values inserted
 * in it (it is not restricted only to counting points), which are then averaged over many snapshots.
 * @details Histogram is built by using add() method, which accumulates a given value in an appropriate bin. It can be
 * done concurrently using many OpenMP threads (as specified in the constructor) - each thread has its own storage.
 * Then nextSnapshot() method adds the data accumulated by add() to the main histogram and increases the snapshot
 * counter. Before adding the snapshot one can renormalize bin values using renormalizeBins() method. After gathering
 * all snapshots dumpHistogram() (or dumoValues()) method can be used to obtain a final, snapshot-averaged histogram.
 */
template<std::size_t DIM = 1, typename T = double>
class HistogramBuilder {
    static_assert(DIM >= 1);

private:
    /* Helper class, which accumulated the value and counts how many points were passed. */
    struct CountingAccumulator {
        T value{};
        std::size_t numPoints{};

        void addPoint(const T &newValue);

        CountingAccumulator &operator+=(const CountingAccumulator &other);
        CountingAccumulator &operator*=(double factor);
    };

    std::array<double, DIM> min{};
    std::array<double, DIM> max{};
    std::array<double, DIM> step{};
    std::array<std::size_t, DIM> numBins{};
    std::size_t flatNumBins{};
    std::size_t numSnapshots{};
    Histogram<DIM, CountingAccumulator> histogram;
    std::vector<Histogram<DIM, CountingAccumulator>> currentHistograms;
    T initialValue{};

    template<typename T1>
    static std::array<std::decay_t<T1>, DIM> filledArray(T1 &&value);

public:
    /**
     * @brief Construct a class where for axis @a i = 0, 1, ... values are gathered in the range `[min[i], max[i]]`
     * (inclusive) divided into `numBins[i]` bins with initial value @a initialValue; it setups concurrent accumulation
     * for at most @a numThreads OpenMP threads.
     * @details If @a numThreads is equal to 0, @a omp_get_max_threads() threads will be used.
     */
    explicit HistogramBuilder(const std::array<double, DIM> &min, const std::array<double, DIM> &max,
                              const std::array<std::size_t, DIM> &numBins, std::size_t numThreads = 1,
                              const T &initialValue = T{});

    /**
     * @brief Simplified version of
     * HistogramBuilder(const std::array<double, DIM>&, const std::array<double, DIM>&, const std::array<std::size_t, DIM>&, std::size_t),
     * where all dimensions have the same range and are divided into the same number of bins.
     */
    explicit HistogramBuilder(double min, double max, std::size_t numBins, std::size_t numThreads = 1,
                              const T &initialValue = T{})
            : HistogramBuilder(filledArray(min), filledArray(max), filledArray(numBins), numThreads,
                               initialValue)
    { }

    /**
     * @brief Adds value @a value to bin enclosing position @a pos. It can be used concurrently by many threads.
     * @details Values are added to current temporary snapshot, which then can be added to the main histogram using
     * nextSnapshot() method. If one wants to create an ordinary histogram with points counting, value should be 1 (or
     * other constant) and then ReductionMethod::SUM should be used while dumping.
     */
    void add(const Vector<DIM> &pos, const T &value);

    /**
     * @brief Simplified version of add(const Vector<DIM>&, const T&) for 1D histogram.
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, void> add(double pos, const T &value) {
        return this->add(Vector<1>{pos}, value);
    }

    /**
     * @brief Saves values added using add() and possibly renormalized using renormalizeBins() as a new snapshot in the
     * main histogram.
     * @details After invocation, current temporary snapshot is cleared.
     */
    void nextSnapshot();

    /**
     * @brief Dumps snapshot-averaged flattened histogram from collected snapshots.
     * @param reductionMethod method used to average over snapshots (see HistogramBuilder::ReductionMethod)
     * @return Vector of BinValue objects enclosing middle position of bin and appropriately averaged value.
     */
    [[nodiscard]] std::vector<typename Histogram<DIM, T>::BinValue> dumpValues(ReductionMethod reductionMethod) const;

    /**
     * @brief Creates snapshot-averaged histogram from collected snapshots.
     * @param reductionMethod method used to average over snapshots (see HistogramBuilder::ReductionMethod)
     * @return Vector of BinValue objects enclosing middle position of bin and appropriately averaged value.
     */
    [[nodiscard]] Histogram<DIM, T> dumpHistogram(ReductionMethod reductionMethod) const;

    /**
     * @brief Clears all snapshots and current temporary snapshot.
     */
    void clear();

    /**
     * @brief See Histogram::getNumBins(std::size_t) const.
     */
    [[nodiscard]] std::size_t getNumBins(std::size_t idx) const { return this->histogram.getNumBins(idx); }

    /**
     * @brief See Histogram::getBinSize(std::size_t) const.
     */
    [[nodiscard]] double getBinSize(std::size_t idx) const { return this->histogram.getBinSize(idx); }

    /**
     * @brief See Histogram::getMin(std::size_t) const.
     */
    [[nodiscard]] double getMin(std::size_t idx) const { return this->histogram.getMin(idx); }

    /**
     * @brief See Histogram::getMax(std::size_t) const.
     */
    [[nodiscard]] double getMax(std::size_t idx) const { return this->histogram.getMax(idx); }

    /**
     * @brief See Histogram::getBinDividers(std::size_t) const.
     */
    [[nodiscard]] std::vector<double> getBinDividers(std::size_t idx) const {
        return this->histogram.getBinDividers(idx);
    }

    /**
     * @brief See Histogram::getNumBins() const.
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, std::size_t> getNumBins() const { return this->histogram.getNumBins(); }

    /**
     * @brief See Histogram::getBinSize() const.
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, double> getBinSize() const { return this->histogram.getBinSize(); }

    /**
     * @brief See Histogram::getMin() const.
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, double> getMin() const { return this->histogram.getMin(); }

    /**
     * @brief See Histogram::getMax() const.
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, double> getMax() const { return this->histogram.getMax(); }

    /**
     * @brief See Histogram::getBinDividers() const.
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, std::vector<double>> getBinDividers() const {
        return this->histogram.getBinDividers();
    }

    /**
     * @brief For a 1D HistogramBuilder, multiplies all bins of the current histogram by corresponding factors from
     * @a factors vector.
     */
    template<std::size_t DIM_ = DIM>
    std::enable_if_t<DIM_ == 1, void> renormalizeBins(const std::vector<double> &factors);
};

using HistogramBuilder1D = HistogramBuilder<1>;
using HistogramBuilder2D = HistogramBuilder<2>;
using HistogramBuilder3D = HistogramBuilder<3>;

#include "HistogramBuilder.tpp"

#endif //RAMPACK_HISTOGRAMBUILDER_H
