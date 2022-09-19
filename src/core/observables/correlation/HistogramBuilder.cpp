//
// Created by pkua on 12.09.22.
//

#include <algorithm>
#include <cmath>

#include "HistogramBuilder.h"
#include "utils/Assertions.h"
#include "utils/OMPMacros.h"


void HistogramBuilder::add(double pos, double value) {
    Expects(pos >= this->min);
    Expects(pos <= this->max);
    std::size_t threadId = _OMP_THREAD_ID;
    Expects(threadId < this->currentHistograms.size());

    auto binIdx = static_cast<std::size_t>(std::floor((pos - this->min)/this->step));
    if (binIdx >= this->size())
        binIdx = this->size() - 1;

    auto &currentHistogram = this->currentHistograms[threadId];
    currentHistogram.bins[binIdx].addPoint(value);
}

void HistogramBuilder::nextSnapshot() {
    for (auto &currentHistogram : this->currentHistograms) {
        this->histogram += currentHistogram;
        currentHistogram.clear();
    }
    this->numSnapshots++;
}

void HistogramBuilder::clear() {
    this->histogram.clear();
    for (auto &currentHistogram : this->currentHistograms)
        currentHistogram.clear();
    this->numSnapshots = 0;
}

std::vector<HistogramBuilder::BinValue>
HistogramBuilder::dumpValues(HistogramBuilder::ReductionMethod reductionMethod) const
{
    std::vector<BinValue> result(this->size());

    if (this->numSnapshots == 0) {
        std::transform(this->binValues.begin(), this->binValues.end(), result.begin(),
                       [](double binMiddle) { return BinValue{binMiddle, 0}; });
        return result;
    }

    switch (reductionMethod) {
        case ReductionMethod::SUM:
            std::transform(this->histogram.bins.begin(), this->histogram.bins.end(), this->binValues.begin(),
                           result.begin(),
                           [this](const BinData &binData, double binValue) {
                                return BinValue{binValue, binData.value / this->numSnapshots};
                           });
            break;

        case ReductionMethod::AVERAGE:
            std::transform(this->histogram.bins.begin(), this->histogram.bins.end(), this->binValues.begin(),
                           result.begin(),
                           [](const BinData &binData, double binValue) {
                               return BinValue{binValue, binData.value / binData.numPoints};
                           });
            break;

        default:
            throw std::runtime_error("");
    }
    return result;
}

HistogramBuilder::HistogramBuilder(double min, double max, std::size_t numBins, std::size_t numThreads)
        : min{min}, max{max}, step{(max - min)/numBins}, histogram(numBins),
          binValues(numBins)
{
    Expects(this->max > this->min);
    Expects(numBins >= 1);

    if (numThreads == 0)
        numThreads = _OMP_MAXTHREADS;
    currentHistograms.resize(numThreads, Histogram{numBins});

    for (std::size_t i{}; i < numBins; i++)
        this->binValues[i] = this->min + (static_cast<double>(i) + 0.5) * this->step;
}

std::vector<double> HistogramBuilder::getBinDividers() const {
    std::vector<double> result(this->size() + 1);
    auto ds = static_cast<double>(this->size());
    for (std::size_t i{}; i <= this->size(); i++) {
        auto di = static_cast<double>(i);
        result[i] = this->min * ((ds - di) / ds) + this->max * (di / ds);
    }
    return result;
}

void HistogramBuilder::renormalizeBins(const std::vector<double> &factors) {
    for (auto &currentHistogram : this->currentHistograms)
        currentHistogram.renormalizeBins(factors);
}

HistogramBuilder::Histogram & HistogramBuilder::Histogram::operator+=(const HistogramBuilder::Histogram &otherData) {
    Expects(otherData.bins.size() == this->bins.size());
    for (std::size_t i{}; i < this->size(); i++)
        this->bins[i] += otherData.bins[i];
    return *this;
}

void HistogramBuilder::Histogram::clear() {
    for (auto &bin : this->bins)
        bin = BinData{};
}

void HistogramBuilder::Histogram::renormalizeBins(const std::vector<double> &factors) {
    Expects(factors.size() == this->bins.size());
    for (std::size_t i{}; i < this->size(); i++)
        this->bins[i] *= factors[i];
}

HistogramBuilder::BinData &HistogramBuilder::BinData::operator+=(const HistogramBuilder::BinData &other) {
    this->value += other.value;
    this->numPoints += other.numPoints;
    return *this;
}

void HistogramBuilder::BinData::addPoint(double newValue) {
    this->value += newValue;
    this->numPoints++;
}

HistogramBuilder::BinData &HistogramBuilder::BinData::operator*=(double factor) {
    this->value *= factor;
    return *this;
}
