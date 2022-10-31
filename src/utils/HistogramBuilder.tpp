//
// Created by pkua on 12.09.22.
//


template<std::size_t DIM>
void HistogramBuilder<DIM>::add(double pos, double value) {
    Expects(pos >= this->min);
    Expects(pos <= this->max);
    std::size_t threadId = _OMP_THREAD_ID;
    Expects(threadId < this->currentHistograms.size());

    auto binIdx = static_cast<std::size_t>(std::floor((pos - this->min)/this->step));
    if (binIdx >= this->getNumBins())
        binIdx = this->getNumBins() - 1;

    auto &currentHistogram = this->currentHistograms[threadId];
    currentHistogram.bins[binIdx].addPoint(value);
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
std::vector<typename HistogramBuilder<DIM>::BinValue>
HistogramBuilder<DIM>::dumpValues(HistogramBuilder::ReductionMethod reductionMethod) const
{
    std::vector<BinValue> result(this->getNumBins());

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
                                return BinValue{binValue, binData.value / static_cast<double>(this->numSnapshots)};
                           });
            break;

        case ReductionMethod::AVERAGE:
            std::transform(this->histogram.bins.begin(), this->histogram.bins.end(), this->binValues.begin(),
                           result.begin(),
                           [](const BinData &binData, double binValue) {
                               return BinValue{binValue, binData.value / static_cast<double>(binData.numPoints)};
                           });
            break;

        default:
            throw std::runtime_error("");
    }
    return result;
}

template<std::size_t DIM>
HistogramBuilder<DIM>::HistogramBuilder(double min, double max, std::size_t numBins, std::size_t numThreads)
        : min{min}, max{max}, step{(max - min)/static_cast<double>(numBins)}, histogram(numBins),
          binValues(numBins)
{
    Expects(this->max > this->min);
    Expects(numBins >= 1);

    if (numThreads == 0)
        numThreads = _OMP_MAXTHREADS;
    this->currentHistograms.resize(numThreads, Histogram{numBins});

    for (std::size_t i{}; i < numBins; i++)
        this->binValues[i] = this->min + (static_cast<double>(i) + 0.5) * this->step;
}

template<std::size_t DIM>
std::vector<double> HistogramBuilder<DIM>::getBinDividers() const {
    std::vector<double> result(this->getNumBins() + 1);
    auto numBinsD = static_cast<double>(this->getNumBins());
    for (std::size_t i{}; i <= this->getNumBins(); i++) {
        auto iD = static_cast<double>(i);
        result[i] = this->min * ((numBinsD - iD) / numBinsD) + this->max * (iD / numBinsD);
    }
    return result;
}

template<std::size_t DIM>
void HistogramBuilder<DIM>::renormalizeBins(const std::vector<double> &factors) {
    for (auto &currentHistogram : this->currentHistograms)
        currentHistogram.renormalizeBins(factors);
}

template<std::size_t DIM>
typename HistogramBuilder<DIM>::Histogram &
HistogramBuilder<DIM>::Histogram::operator+=(const HistogramBuilder::Histogram &otherData)
{
    Expects(otherData.bins.size() == this->bins.size());
    for (std::size_t i{}; i < this->size(); i++)
        this->bins[i] += otherData.bins[i];
    return *this;
}

template<std::size_t DIM>
void HistogramBuilder<DIM>::Histogram::clear() {
    for (auto &bin : this->bins)
        bin = BinData{};
}

template<std::size_t DIM>
void HistogramBuilder<DIM>::Histogram::renormalizeBins(const std::vector<double> &factors) {
    Expects(factors.size() == this->bins.size());
    for (std::size_t i{}; i < this->size(); i++)
        this->bins[i] *= factors[i];
}

template<std::size_t DIM>
typename HistogramBuilder<DIM>::BinData &
HistogramBuilder<DIM>::BinData::operator+=(const HistogramBuilder::BinData &other)
{
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
