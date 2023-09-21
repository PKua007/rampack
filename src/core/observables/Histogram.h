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


/**
 * @brief Class representing a @a DIM -dimensional histogram.
 *
 * @tparam DIM dimensionality of the histogram
 * @tparam T type of the value stored inside the histogram. It is required to support the following operators:
 * @code
 * T::operator+=(const T&)
 * T::operator*=(double)
 * T::operator/=(double)
 * @endcode
 */
template<std::size_t DIM = 1, typename T = double>
struct Histogram {
public:
    struct ValueCount {
        T value{};
        std::size_t count{};

        ValueCount &operator+=(const ValueCount &vc) {
            this->value += vc.value;
            this->count += vc.count;
            return *this;
        }

        template <typename U>
        ValueCount &operator*=(const U &u) {
            this->value *= u;
            return *this;
        }

        template <typename U>
        ValueCount &operator/=(const U &u) {
            this->value /= u;
            return *this;
        }

        friend ValueCount operator+(const ValueCount &lhs, const ValueCount &rhs) {
            return ValueCount(lhs) += rhs;
        }

        template <typename U>
        friend ValueCount operator*(const ValueCount &lhs, const U &rhs) {
            return ValueCount(lhs) *= rhs;
        }

        template <typename U>
        friend ValueCount operator/(const ValueCount &lhs, const U &rhs) {
            return ValueCount(lhs) /= rhs;
        }

        friend bool operator==(const ValueCount &lhs, const ValueCount &rhs) {
            return std::tie(lhs.value, lhs.count) == std::tie(rhs.value, rhs.count);
        }

        friend bool operator!=(const ValueCount &lhs, const ValueCount &rhs) {
            return !(rhs == lhs);
        }

        friend std::ostream &operator<<(std::ostream &os, const ValueCount &valueCount) {
            os << "{value: " << valueCount.value << ", count: " << valueCount.count << "}";
            return os;
        }
    };

private:
    std::array<double, DIM> min{};
    std::array<double, DIM> max{};
    std::array<double, DIM> step{};
    std::array<std::size_t, DIM> numBins;
    std::vector<ValueCount> bins;
    T initialValue{};

    [[nodiscard]] std::array<std::size_t, DIM> calculateBinIndex(const Vector<DIM> &pos) const;
    [[nodiscard]] std::size_t calculateFlatBinIndex(const Vector<DIM> &pos) const;
    [[nodiscard]] std::size_t flattenIndex(const std::array<std::size_t, DIM> &index) const;
    [[nodiscard]] std::vector<Vector<DIM>> dumpBinMiddles() const;
    [[nodiscard]] std::array<std::size_t, DIM> reshapeIndex(std::size_t flatIdx) const;

public:
    using iterator = typename decltype(bins)::iterator;
    using const_iterator = typename decltype(bins)::const_iterator;

    /**
     * @brief A single bin data consisting of Vector pointing to bin's middle and the in value.
     */
    struct BinValue {
        /** @brief Position corresponding to the middle of the bin. */
        Vector<DIM> binMiddle{};
        /** @brief Value of the bin. */
        T value{};
        std::size_t count{};

        BinValue(const Vector<DIM> &binMiddle, T value, std::size_t count = 0)
                : binMiddle{binMiddle}, value{std::move(value)}, count{count}
        { }

        template<std::size_t DIM_ = DIM>
        BinValue(std::enable_if_t<DIM_ == 1, double> binMiddle, T value, std::size_t count = 0)
                : binMiddle{binMiddle}, value{std::move(value)}, count{count}
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

    
    /**
     * @brief Created the histogram, where for axis @a i = 0, 1, ... values are from in the range `[min[i], max[i]]`
     * (inclusive) divided into `numBins[i]` bins, with initial value given by @a initialValue.
     */
    explicit Histogram(const std::array<double, DIM> &min, const std::array<double, DIM> &max,
                       const std::array<std::size_t, DIM> &numBins, const T &initialValue = T{});

    /**
     * @brief Sums the histogram bin-wise with other histogram @a other.
     */
    Histogram &operator+=(const Histogram &other);

    /**
     * @brief Multiplies all bin values by @a val value.
     */
    Histogram &operator*=(double val);

    /**
     * @brief Divides all bin values by @a val value.
     */
    Histogram &operator/=(double val);

    /**
     * @brief Returns immutable bin value for a bin with multidimensional index @a idx.
     */
    [[nodiscard]] const ValueCount &atIndex(const std::array<std::size_t, DIM> &idx) const;

    /**
     * @brief Returns mutable bin value for a bin with multidimensional index @a idx.
     */
    [[nodiscard]] ValueCount &atIndex(const std::array<std::size_t, DIM> &idx) {
        return const_cast<ValueCount&>(std::as_const(*this).atIndex(idx));
    }

    /**
     * @brief Returns immutable bin value for a bin which contains the position @a pos.
     */
    [[nodiscard]] const ValueCount &atPos(const Vector<DIM> &pos) const;

    /**
     * @brief Returns mutable bin value for a bin which contains the position @a pos.
     */
    [[nodiscard]] ValueCount &atPos(const Vector<DIM> &pos) {
        return const_cast<ValueCount&>(std::as_const(*this).atPos(pos));
    }

    [[nodiscard]] std::size_t size() const { return bins.size(); }
    void clear();

    [[nodiscard]] iterator begin() { return this->bins.begin(); }
    [[nodiscard]] iterator end() { return this->bins.end(); }
    [[nodiscard]] const_iterator begin() const { return this->bins.begin(); }
    [[nodiscard]] const_iterator end() const { return this->bins.end(); }

    /**
     * @brief Dumps flattened histogram.
     */
    [[nodiscard]] std::vector<BinValue> dumpValues() const;

    /**
     * @brief For a 1D histogram, multiplies all bins by corresponding factors from @a factors vector.
     */
    template< std::size_t DIM_ = DIM>
    std::enable_if_t<DIM_ == 1, void> renormalizeBins(const std::vector<double> &factors);

    /**
     * @brief Returns the number of bins for a given axis with index @a idx.
     */
    [[nodiscard]] std::size_t getNumBins(std::size_t idx) const { return this->numBins.at(idx); }

    /**
     * @brief Returns the number of bins (simplified version for 1D histogram).
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, std::size_t> getNumBins() const { return this->numBins.front(); }

    /**
     * @brief Returns the width of the bin for a given axis with index @a idx.
     */
    [[nodiscard]] double getBinSize(std::size_t idx) const { return this->step.at(idx); }

    /**
     * @brief Returns the width of the bin (simplified version for 1D histogram).
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, double> getBinSize() const { return this->step.front(); }

    /**
     * @brief Returns a lower end of binned range for a given axis with index @a idx.
     */
    [[nodiscard]] double getMin(std::size_t idx) const { return this->min.at(idx); }

    /**
     * @brief Returns a lower end of binned range (simplified version for 1D histogram).
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, double> getMin() const { return this->min.front(); }

    /**
     * @brief Returns an upper end of binned range for a given axis with index @a idx.
     */
    [[nodiscard]] double getMax(std::size_t idx) const { return this->max.at(idx); }

    /**
     * @brief Returns an upper end of binned range (simplified version for 1D histogram).
     */
    template<std::size_t DIM_ = DIM>
    [[nodiscard]] std::enable_if_t<DIM_ == 1, double> getMax() const { return this->max.front(); }

    /**
     * @brief Return a sorted vector of getNumBins() + 1 positions of bin boundaries including getMin() and getMax()
     * for a given axis with index @a idx.
     */
    [[nodiscard]] std::vector<double> getBinDividers(std::size_t idx) const;

    /**
     * @brief Return a sorted vector of getNumBins() + 1 positions of bin boundaries including getMin() and getMax()
     * (simplified version for 1D histogram).
     */
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
