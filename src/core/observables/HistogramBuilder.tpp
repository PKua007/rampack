//
// Created by pkua on 12.09.22.
//

#include <tuple>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "ZipIterator.hpp"

#include "utils/Exceptions.h"
#include "utils/OMPMacros.h"
#include "HistogramBuilder.h"


template<std::size_t DIM, typename T>
Histogram<DIM, T>::Histogram(const std::array<double, DIM> &min, const std::array<double, DIM> &max,
                             const std::array<std::size_t, DIM> &numBins)
        : min{min}, max{max}, numBins{numBins},
          bins(std::accumulate(numBins.begin(), numBins.end(), 1., std::multiplies<>{}))
{
    for (auto[maxItem, minItem] : Zip(max, min))
        Expects(maxItem > minItem);
    Expects(this->size() >= 1);

    for (std::size_t i{}; i < DIM; i++)
        this->step[i] = (max[i] - min[i]) / static_cast<double>(numBins[i]);
}

template<std::size_t DIM, typename T>
Histogram<DIM, T> &Histogram<DIM, T>::operator+=(const Histogram &otherData)
{
    Expects(otherData.bins.size() == this->bins.size());
    for (std::size_t i{}; i < this->size(); i++)
        this->bins[i] += otherData.bins[i];
    return *this;
}

template<std::size_t DIM, typename T>
void Histogram<DIM, T>::clear() {
    for (auto &bin : this->bins)
        bin = T{};
}

template<std::size_t DIM, typename T>
template<std::size_t DIM_>
std::enable_if_t<DIM_ == 1, void>
Histogram<DIM, T>::renormalizeBins(const std::vector<double> &factors)
{
    Expects(factors.size() == this->bins.size());
    for (std::size_t i{}; i < this->size(); i++)
        this->bins[i] *= factors[i];
}

template<std::size_t DIM, typename T>
std::array<std::size_t, DIM> Histogram<DIM, T>::calculateBinIndex(const Vector<DIM> &pos) const {
    std::array<std::size_t, DIM> idx{};
    for (std::size_t i{}; i < DIM; i++) {
        idx[i] = static_cast<std::size_t>(std::floor((pos[i] - this->min[i]) / this->step[i]));
        if (idx[i] >= this->numBins[i])
            idx[i] = this->numBins[i] - 1;
    }
    return idx;
}

template<std::size_t DIM, typename T>
std::size_t Histogram<DIM, T>::flattenIndex(const std::array<std::size_t, DIM> &index) const {
    std::size_t flatIdx{};
    for (auto [numBinsItem, indexItem] : Zip(this->numBins, index))
        flatIdx = numBinsItem*flatIdx + indexItem;
    return flatIdx;
}

template<std::size_t DIM, typename T>
std::size_t Histogram<DIM, T>::calculateFlatBinIndex(const Vector<DIM> &pos) const {
    return this->flattenIndex(this->calculateBinIndex(pos));
}

template<std::size_t DIM, typename T>
const T &Histogram<DIM, T>::atPos(const Vector<DIM> &pos) const {
    for (auto[posItem, minItem, maxItem] : Zip(pos, this->min, this->max)) {
        Expects(posItem >= minItem);
        Expects(posItem <= maxItem);
    }

    auto binIdx = this->calculateFlatBinIndex(pos);
    if (binIdx >= this->size())
        binIdx = this->size() - 1;

    return this->bins[binIdx];
}

template<std::size_t DIM, typename T>
const T &Histogram<DIM, T>::atIndex(const std::array<std::size_t, DIM> &idx) const {
    for (auto[idxItem, numBinsItem] : Zip(idx, this->numBins))
        Expects(idxItem < numBinsItem);

    auto binIdx = this->flattenIndex(idx);
    return this->bins[binIdx];
}


//----------------------------------


template<std::size_t DIM>
std::array<std::size_t, DIM> HistogramBuilder<DIM>::reshapeIndex(std::size_t flatIdx) const {
    std::array<std::size_t, DIM> idx{};
    for (int i = DIM - 1; i >= 0; i--) {
        idx[i] = flatIdx % this->numBins[i];
        flatIdx /= this->numBins[i];
    }
    return idx;
}


template<std::size_t DIM>
void HistogramBuilder<DIM>::add(const Vector<DIM> &pos, double value) {
    std::size_t threadId = OMP_THREAD_ID;
    Expects(threadId < this->currentHistograms.size());
    auto &currentHistogram = this->currentHistograms[threadId];
    currentHistogram.atPos(pos).addPoint(value);
}

template<std::size_t DIM>
void HistogramBuilder<DIM>::nextSnapshot() {
    for (auto &currentHistogram : this->currentHistograms) {
        this->histogram += currentHistogram;
        currentHistogram.clear();
    }
    this->numSnapshots++;
}

template<std::size_t DIM>
void HistogramBuilder<DIM>::clear() {
    this->histogram.clear();
    for (auto &currentHistogram : this->currentHistograms)
        currentHistogram.clear();
    this->numSnapshots = 0;
}

template<std::size_t DIM>
std::vector<BinValue<DIM>>
HistogramBuilder<DIM>::dumpValues(HistogramBuilder::ReductionMethod reductionMethod) const
{
    std::vector<Vector<DIM>> binValues(this->flatNumBins);
    for (std::size_t i{}; i < this->flatNumBins; i++) {
        auto idx = this->reshapeIndex(i);
        for (std::size_t j{}; j < DIM; j++)
            binValues[i][j] = this->min[j] + (static_cast<double>(idx[j]) + 0.5) * this->step[j];
    }

    auto hist = this->dumpHistogram(reductionMethod);
    std::vector<BinValue<DIM>> result;
    result.reserve(this->flatNumBins);
    std::transform(hist.begin(), hist.end(), binValues.begin(), std::back_inserter(result),
                   [this](double binData, const Vector<DIM> &binValue) {
                        return BinValue{binValue, binData};
                   });
    return result;
}

template<std::size_t DIM>
Histogram<DIM> HistogramBuilder<DIM>::dumpHistogram(HistogramBuilder::ReductionMethod reductionMethod) const {
    Histogram<DIM> result(this->min, this->max, this->numBins);

    if (this->numSnapshots == 0)
        return result;

    switch (reductionMethod) {
        case ReductionMethod::SUM:
            std::transform(this->histogram.begin(), this->histogram.end(), result.begin(),
                           [this](const BinData &binData) {
                               return binData.value / static_cast<double>(this->numSnapshots);
                           });
            break;

        case ReductionMethod::AVERAGE:
            std::transform(this->histogram.begin(), this->histogram.end(), result.begin(),
                           [](const BinData &binData) {
                               return binData.value / static_cast<double>(binData.numPoints);
                           });
            break;

        default:
            throw std::runtime_error("");
    }
    return result;
}

template<std::size_t DIM>
HistogramBuilder<DIM>::HistogramBuilder(const std::array<double, DIM> &min, const std::array<double, DIM> &max,
                                        const std::array<std::size_t, DIM> &numBins, std::size_t numThreads)
        : min{min}, max{max}, numBins{numBins},
          flatNumBins{std::accumulate(numBins.begin(), numBins.end(), 1ul, std::multiplies<>{})},
          histogram(min, max, numBins)
{
    for (auto[maxItem, minItem] : Zip(max, min))
        Expects(maxItem > minItem);
    Expects(this->flatNumBins >= 1);

    for (std::size_t i{}; i < DIM; i++)
        this->step[i] = (max[i] - min[i]) / static_cast<double>(numBins[i]);

    if (numThreads == 0)
        numThreads = OMP_MAXTHREADS;
    this->currentHistograms.resize(numThreads, Histogram<DIM, BinData>(this->min, this->max, this->numBins));
}

template<std::size_t DIM>
std::vector<double>HistogramBuilder<DIM>::getBinDividers(std::size_t idx) const {
    Expects(idx < DIM);

    std::vector<double> result(this->numBins[idx] + 1);
    auto numBinsD = static_cast<double>(this->numBins[idx]);
    for (std::size_t i{}; i <= this->numBins[idx]; i++) {
        auto iD = static_cast<double>(i);
        result[i] = this->min[idx] * ((numBinsD - iD) / numBinsD) + this->max[idx] * (iD / numBinsD);
    }
    return result;
}

template<std::size_t DIM>
template<std::size_t DIM_>
std::enable_if_t<DIM_ == 1, void> HistogramBuilder<DIM>::renormalizeBins(const std::vector<double> &factors) {
    for (auto &currentHistogram : this->currentHistograms)
        currentHistogram.renormalizeBins(factors);
}

template<std::size_t DIM>
template<typename T>
std::array<std::decay_t<T>, DIM> HistogramBuilder<DIM>::filledArray(T &&value) {
    std::array<std::remove_reference_t<T>, DIM> array;
    array.fill(std::forward<T>(value));
    return array;
}

template<std::size_t DIM>
typename HistogramBuilder<DIM>::BinData &
HistogramBuilder<DIM>::BinData::operator+=(const HistogramBuilder::BinData &other) {
    this->value += other.value;
    this->numPoints += other.numPoints;
    return *this;
}

template<std::size_t DIM>
void HistogramBuilder<DIM>::BinData::addPoint(double newValue) {
    this->value += newValue;
    this->numPoints++;
}

template<std::size_t DIM>
typename HistogramBuilder<DIM>::BinData &HistogramBuilder<DIM>::BinData::operator*=(double factor) {
    this->value *= factor;
    return *this;
}
