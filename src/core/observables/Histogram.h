//
// Created by Piotr Kubala on 21/04/2023.
//

#ifndef RAMPACK_HISTOGRAM_H
#define RAMPACK_HISTOGRAM_H

#include <array>
#include <vector>
#include <ostream>
#include <ZipIterator.hpp>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <tuple>

#include "utils/OMPMacros.h"
#include "utils/Exceptions.h"
#include "geometry/Vector.h"


template<std::size_t DIM = 1, typename T = double>
struct Histogram {
private:
    std::array<double, DIM> min{};
    std::array<double, DIM> max{};
    std::array<double, DIM> step{};
    std::array<std::size_t, DIM> numBins;
    std::vector<T> bins;

    [[nodiscard]] std::array<std::size_t, DIM> calculateBinIndex(const Vector<DIM> &pos) const;
    [[nodiscard]] std::size_t calculateFlatBinIndex(const Vector<DIM> &pos) const;
    [[nodiscard]] std::size_t flattenIndex(const std::array<std::size_t, DIM> &index) const;
    [[nodiscard]] std::vector<Vector<DIM>> dumpBinMiddles() const;
    [[nodiscard]] std::array<std::size_t, DIM> reshapeIndex(std::size_t flatIdx) const;

public:
    using iterator = typename decltype(bins)::iterator;
    using const_iterator = typename decltype(bins)::const_iterator;

    /**
     * @brief A single bin data.
     */
    struct BinValue {
        /** @brief Position corresponding to the middle of the bin. */
        Vector<DIM> binMiddle{};
        /** @brief Value of the bin. */
        T value{};

        BinValue() = default;
        BinValue(const Vector<DIM> &binMiddle, T value) : binMiddle{binMiddle}, value{std::move(value)} { }

        template<std::size_t DIM_ = DIM>
        BinValue(std::enable_if_t<DIM_ == 1, double> binMiddle, T value) : binMiddle{binMiddle}, value{std::move(value)}
        { }

        friend bool operator==(const BinValue &lhs, const BinValue &rhs) {
            return std::tie(lhs.binMiddle, lhs.value) == std::tie(rhs.binMiddle, rhs.value);
        }

        friend bool operator!=(const BinValue &lhs, const BinValue &rhs) {
            return !(rhs == lhs);
        }

        /**
         * @brief Stream insertion operator printing the bin in the form "[bin middle] [bin value]".
         */
        friend std::ostream &operator<<(std::ostream &out, const BinValue &bin) {
            return out << bin.binMiddle << " " << bin.value;
        }
    };


    explicit Histogram(const std::array<double, DIM> &min, const std::array<double, DIM> &max,
                       const std::array<std::size_t, DIM> &numBins);

    Histogram &operator+=(const Histogram &otherData);

    template<typename T1>
    Histogram &operator*=(const T1 &val);

    template<typename T1>
    Histogram &operator/=(const T1 &val);

    [[nodiscard]] const T &atIndex(const std::array<std::size_t, DIM> &idx) const;

    [[nodiscard]] T &atIndex(const std::array<std::size_t, DIM> &idx) {
        return const_cast<T&>(std::as_const(*this).atIndex(idx));
    }

    [[nodiscard]] const T &atPos(const Vector<DIM> &pos) const;

    [[nodiscard]] T &atPos(const Vector<DIM> &pos) {
        return const_cast<T&>(std::as_const(*this).atPos(pos));
    }

    [[nodiscard]] std::size_t size() const { return bins.size(); }
    void clear();

    [[nodiscard]] iterator begin() { return this->bins.begin(); }
    [[nodiscard]] iterator end() { return this->bins.end(); }
    [[nodiscard]] const_iterator begin() const { return this->bins.begin(); }
    [[nodiscard]] const_iterator end() const { return this->bins.end(); }

    [[nodiscard]] std::vector<BinValue> dumpValues() const;

    template<std::size_t DIM_ = DIM>
    std::enable_if_t<DIM_ == 1, void> renormalizeBins(const std::vector<double> &factors);

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
};

using Histogram1D = Histogram<1, double>;
using Histogram2D = Histogram<2, double>;
using Histogram3D = Histogram<3, double>;

#include "Histogram.tpp"

#endif //RAMPACK_HISTOGRAM_H
