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


template<std::size_t DIM, typename T>
void HistogramBuilder<DIM, T>::add(const Vector<DIM> &pos, const T &value) {
    std::size_t threadId = OMP_THREAD_ID;
    Expects(threadId < this->currentHistograms.size());
    auto &currentHistogram = this->currentHistograms[threadId];
    currentHistogram.atPos(pos).addPoint(value);
}

template<std::size_t DIM, typename T>
void HistogramBuilder<DIM, T>::nextSnapshot() {
    for (auto &currentHistogram : this->currentHistograms) {
        this->histogram += currentHistogram;
        currentHistogram.clear();
    }
    this->numSnapshots++;
}

template<std::size_t DIM, typename T>
void HistogramBuilder<DIM, T>::clear() {
    this->histogram.clear();
    for (auto &currentHistogram : this->currentHistograms)
        currentHistogram.clear();
    this->numSnapshots = 0;
}

template<std::size_t DIM, typename T>
std::vector<typename Histogram<DIM, T>::BinValue>
HistogramBuilder<DIM, T>::dumpValues(ReductionMethod reductionMethod) const {
    return this->dumpHistogram(reductionMethod).dumpValues();
}

template<std::size_t DIM, typename T>
Histogram<DIM, T> HistogramBuilder<DIM, T>::dumpHistogram(ReductionMethod reductionMethod) const {
    Histogram<DIM, T> result(this->min, this->max, this->numBins, this->initialValue);

    if (this->numSnapshots == 0)
        return result;

    switch (reductionMethod) {
        case ReductionMethod::SUM:
            std::transform(this->histogram.begin(), this->histogram.end(), result.begin(),
                           [this](const CountingAccumulator &binData) {
                               return binData.value / static_cast<double>(this->numSnapshots);
                           });
            break;

        case ReductionMethod::AVERAGE:
            std::transform(this->histogram.begin(), this->histogram.end(), result.begin(),
                           [](const CountingAccumulator &binData) {
                               return binData.value / static_cast<double>(binData.numPoints);
                           });
            break;

        default:
            throw std::runtime_error("");
    }
    return result;
}

template<std::size_t DIM, typename T>
HistogramBuilder<DIM, T>::HistogramBuilder(const std::array<double, DIM> &min, const std::array<double, DIM> &max,
                                           const std::array<std::size_t, DIM> &numBins,
                                           std::size_t numThreads, const T &initialValue)
        : min{min}, max{max}, numBins{numBins},
          flatNumBins{std::accumulate(numBins.begin(), numBins.end(), 1ul, std::multiplies<>{})},
          histogram(min, max, numBins, {initialValue}),
          initialValue{initialValue}
{
    for (auto[maxItem, minItem] : Zip(max, min))
        Expects(maxItem > minItem);
    Expects(this->flatNumBins >= 1);

    for (std::size_t i{}; i < DIM; i++)
        this->step[i] = (max[i] - min[i]) / static_cast<double>(numBins[i]);

    if (numThreads == 0)
        numThreads = OMP_MAXTHREADS;
    this->currentHistograms.resize(
        numThreads, Histogram<DIM, CountingAccumulator>(this->min, this->max, this->numBins, {initialValue})
    );
}

template<std::size_t DIM, typename T>
template<std::size_t DIM_>
std::enable_if_t<DIM_ == 1, void> HistogramBuilder<DIM, T>::renormalizeBins(const std::vector<double> &factors) {
    for (auto &currentHistogram : this->currentHistograms)
        currentHistogram.renormalizeBins(factors);
}

template<std::size_t DIM, typename T>
template<typename T1>
std::array<std::decay_t<T1>, DIM> HistogramBuilder<DIM, T>::filledArray(T1 &&value) {
    std::array<std::decay_t<T1>, DIM> array;
    array.fill(std::forward<T1>(value));
    return array;
}

template<std::size_t DIM, typename T>
typename HistogramBuilder<DIM, T>::CountingAccumulator &
HistogramBuilder<DIM, T>::CountingAccumulator::operator+=(const HistogramBuilder::CountingAccumulator &other) {
    this->value += other.value;
    this->numPoints += other.numPoints;
    return *this;
}

template<std::size_t DIM, typename T>
void HistogramBuilder<DIM, T>::CountingAccumulator::addPoint(const T &newValue) {
    this->value += newValue;
    this->numPoints++;
}

template<std::size_t DIM, typename T>
typename HistogramBuilder<DIM, T>::CountingAccumulator &HistogramBuilder<DIM, T>::CountingAccumulator::operator*=(double factor) {
    this->value *= factor;
    return *this;
}
