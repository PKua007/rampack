//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_HISTOGRAMBUILDER_H
#define RAMPACK_HISTOGRAMBUILDER_H

#include <ostream>
#include <vector>
#include <array>

#include "geometry/Vector.h"


/**
 * @brief Class facilitating multithreaded building a histogram, where each bin accumulates arbitrary values inserted
 * in it (it is not restricted only to counting points), which are then averaged over many snapshots.
 * @details Histogram is built by using add() method, which accumulates a given value in an appropriate bin. It can be
 * done concurrently using many OpenMP threads (as specified in the constructor) - each thread has its own storage.
 * Then nextSnapshot() method adds the data accumulated by add() to the main histogram and increases the snapshot
 * counter. Before adding the snapshot one can renormalize bin values using renormalizeBins() method. After gathering
 * all snapshots dumpValues() method can be used to obtain a final, snapshot-averaged histogram.
 */
template<std::size_t DIM = 1>
class HistogramBuilder {
    static_assert(DIM >= 1);

private:
    struct BinData {
        double value{};
        std::size_t numPoints{};

        void addPoint(double newValue);

        BinData &operator+=(const BinData &other);
        BinData &operator*=(double factor);
    };

    struct Histogram {
        std::vector<BinData> bins;

        explicit Histogram(std::size_t numBins) : bins(numBins) { }

        Histogram &operator+=(const Histogram &otherData);

        template<std::size_t DIM_ = DIM>
        std::enable_if_t<DIM_ == 1, void> renormalizeBins(const std::vector<double> &factors);

        [[nodiscard]] std::size_t size() const { return bins.size(); }
        void clear();
    };

    std::array<double, DIM> min{};
    std::array<double, DIM> max{};
    std::array<double, DIM> step{};
    std::array<std::size_t, DIM> numBins{};
    std::size_t flatNumBins{};
    std::size_t numSnapshots{};
    Histogram histogram;
    std::vector<Histogram> currentHistograms;
    std::vector<Vector<DIM>> binValues{};

    template<typename T>
    static std::array<std::remove_reference_t<T>, DIM> filledArray(T &&value);

    [[nodiscard]] std::array<std::size_t, DIM> calculateBinIndex(const Vector<DIM> &pos) const;
    [[nodiscard]] std::size_t calculateFlatBinIndex(const Vector<DIM> &pos) const;
    [[nodiscard]] std::size_t flattenIndex(const std::array<std::size_t, DIM> &index) const;
    [[nodiscard]] std::array<std::size_t, DIM> reshapeIndex(std::size_t flatIdx) const;

public:
    /**
     * @brief A single bin data.
     */
    struct BinValue {
        /** @brief Position corresponding to the middle of the bin. */
        Vector<DIM> binMiddle{};
        /** @brief Value of the bin. */
        double value{};

        BinValue() = default;
        BinValue(const Vector<DIM> &binMiddle, double value) : binMiddle{binMiddle}, value{value} { }

        template<std::size_t DIM_ = DIM>
        BinValue(std::enable_if_t<DIM_ == 1, double> binMiddle, double value) : binMiddle{binMiddle}, value{value} { }

        friend bool operator==(const BinValue &lhs, const BinValue &rhs) {
            return std::tie(lhs.binMiddle, lhs.value) == std::tie(rhs.binMiddle, rhs.value);
        }

        friend bool operator!=(const BinValue &lhs, const BinValue &rhs) {
            return !(rhs == lhs);
        }

        /**
         * @brief Stream insertion operator printing the bin in the form "[bin middle]  space[bin value]".
         */
        friend std::ostream &operator<<(std::ostream &out, const BinValue &bin) {
            return out << bin.binMiddle << " " << bin.value;
        }
    };

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
     * @brief Construct a class to gather values from the range [@a min, @a max] (inclusive) divided into @a numBins
     * bins and enables concurrent accumulation by at most @a numThreads OpenMP threads.
     * @details If @a numThreads is equal to 0, @a omp_get_max_threads() threads will be used.
     */
    explicit HistogramBuilder(const std::array<double, DIM> &min, const std::array<double, DIM> &max,
                              const std::array<std::size_t, DIM> &numBins, std::size_t numThreads = 1);

    explicit HistogramBuilder(double min, double max, std::size_t numBins, std::size_t numThreads = 1)
            : HistogramBuilder(filledArray(min), filledArray(max), filledArray(numBins), numThreads)
    { }

    /**
     * @brief Adds value @a value to bin enclosing position @a pos. It can be used concurrently by many threads.
     * @details Values are added to current temporary snapshot, which then can be added to the main histogram using
     * nextSnapshot() method. If one wants to create an ordinary histogram with points counting, value should be 1 (or
     * other constant) and then ReductionMethod::SUM should be used while dumping.
     */
    void add(const Vector<DIM> &pos, double value);

    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, void> add(double pos, double value) {
        return this->add(Vector<1>{pos}, value);
    }

    /**
     * @brief Saves values added using add() and possibly renormalized using renormalizeBins() as a new snapshot in the
     * main histogram.
     * @details After invocation, current temporary snapshot is cleared.
     */
    void nextSnapshot();

    /**
     * @brief Creates snapshot-averaged histogram from collected snapshots.
     * @param reductionMethod method used to average over snapshots (see HistogramBuilder::ReductionMethod)
     * @return Vector of BinValue objects enclosing middle position of bin and appropriately averaged value.
     */
    [[nodiscard]] std::vector<BinValue> dumpValues(ReductionMethod reductionMethod) const;

    /**
     * @brief Clears all snapshots and current temporary snapshot.
     */
    void clear();

    [[nodiscard]] std::size_t getNumBins(std::size_t idx) const { return this->numBins.at(idx); }

    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, std::size_t> getNumBins() const { return this->numBins.front(); }

    /**
     * @brief Returns the width of the bin.
     */
    [[nodiscard]] double getBinSize(std::size_t idx) const { return this->step.at(idx); }

    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, double> getBinSize() const { return this->step.front(); }

    /**
     * @brief Returns a lower end of binned range.
     */
    [[nodiscard]] double getMin(std::size_t idx) const { return this->min.at(idx); }

    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, double> getMin() const { return this->min.front(); }

    /**
     * @brief Returns an upper end of binned range.
     */
    [[nodiscard]] double getMax(std::size_t idx) const { return this->max.at(idx); }

    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, double> getMax() const { return this->max.front(); }

    /**
     * @brief Return a sorted vector of getNumBins() + 1 positions of bin boundaries including getMin() and getMax().
     */
    [[nodiscard]] std::vector<double> getBinDividers(std::size_t idx) const;

    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, std::vector<double>> getBinDividers() const {
        return this->getBinDividers(0);
    }

    /**
     * @brief Multiplies values in a current snapshot in each bin by corresponding element of vector @a factors.
     */
    template<std::size_t DIM_ = DIM>
    std::enable_if_t<DIM_ == 1, void> renormalizeBins(const std::vector<double> &factors);
};

using HistogramBuilder1D = HistogramBuilder<1>;
using HistogramBuilder2D = HistogramBuilder<2>;
using HistogramBuilder3D = HistogramBuilder<3>;

#include "HistogramBuilder.tpp"

#endif //RAMPACK_HISTOGRAMBUILDER_H
