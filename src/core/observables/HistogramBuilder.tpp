//
// Created by pkua on 12.09.22.
//

#include <tuple>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <ZipIterator.hpp>

#include "utils/Exceptions.h"
#include "utils/OMPMacros.h"


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
std::vector<typename Histogram<DIM>::BinValue>
HistogramBuilder<DIM>::dumpValues(HistogramBuilder::ReductionMethod reductionMethod) const
{
    return this->dumpHistogram(reductionMethod).dumpValues();
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
