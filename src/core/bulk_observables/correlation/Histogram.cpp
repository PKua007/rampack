//
// Created by pkua on 12.09.22.
//

#include <algorithm>
#include <cmath>

#include "Histogram.h"
#include "utils/Assertions.h"


void Histogram::add(double value, double pos) {
    Expects(pos >= this->min);
    Expects(pos <= this->max);

    auto binIdx = static_cast<std::size_t>(std::floor((value - this->min)/this->step));
    if (binIdx < this->size())
        binIdx = this->size() - 1;

    this->currentHistogram.bins[binIdx].addPoint(value);
}

void Histogram::nextSnapshot() {
    this->histogram += this->currentHistogram;
    this->numSnapshots++;
    this->currentHistogram.clear();
}

void Histogram::clear() {
    this->histogram.clear();
    this->currentHistogram.clear();
    this->numSnapshots = 0;
}

std::vector<std::pair<double, double>> Histogram::dumpValues(Histogram::ReductionMethod reductionMethod) const {
    std::vector<std::pair<double, double>> result(this->size());
    switch (reductionMethod) {
        case ReductionMethod::SUM:
            std::transform(this->histogram.bins.begin(), this->histogram.bins.end(), this->binValues.begin(),
                           result.begin(),
                           [this](const BinData &binData, double binValue) {
                                return std::pair{binValue, binData.value / this->numSnapshots};
                           });
            break;

        case ReductionMethod::AVERAGE:
            std::transform(this->histogram.bins.begin(), this->histogram.bins.end(), this->binValues.begin(),
                           result.begin(),
                           [](const BinData &binData, double binValue) {
                               return std::pair{binValue, binData.value / binData.numPoints};
                           });
            break;

        default:
            throw std::runtime_error("");
    }
    return result;
}

Histogram::Histogram(double min, double max, std::size_t numBins)
        : min{min}, max{max}, step{(max - min)/numBins}, histogram(numBins), currentHistogram(numBins),
          binValues(numBins)
{
    Expects(this->max > this->min);
    Expects(numBins >= 1);

    for (std::size_t i{}; i < numBins; i++)
        this->binValues[i] = this->min + (static_cast<double>(i) + 0.5) * this->step;
}

Histogram::HistogramData &Histogram::HistogramData::operator+=(const Histogram::HistogramData &otherData) {
    Expects(otherData.bins.size() == this->bins.size());
    for (std::size_t i{}; i < this->size(); i++)
        this->bins[i] += otherData.bins[i];
    return *this;
}

void Histogram::HistogramData::clear() {
    for (auto &bin : this->bins)
        bin = BinData{};
}

Histogram::BinData &Histogram::BinData::operator+=(const Histogram::BinData &other) {
    this->value += other.value;
    this->numPoints += other.numPoints;
    return *this;
}

void Histogram::BinData::addPoint(double newValue) {
    this->value += newValue;
    this->numPoints++;
}
