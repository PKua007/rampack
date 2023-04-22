//
// Created by Piotr Kubala on 21/04/2023.
//


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
template<typename T1>
Histogram<DIM, T> &Histogram<DIM, T>::operator*=(const T1 &val) {
    for (auto &bin : this->bins)
        bin *= val;
    return *this;
}

template<std::size_t DIM, typename T>
template<typename T1>
Histogram<DIM, T> &Histogram<DIM, T>::operator/=(const T1 &val) {
    for (auto &bin : this->bins)
        bin /= val;
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

template<std::size_t DIM, typename T>
std::vector<Vector<DIM>> Histogram<DIM, T>::dumpBinMiddles() const {
    std::vector<Vector<DIM>> binMiddles(this->size());
    for (std::size_t i{}; i < this->size(); i++) {
        auto idx = this->reshapeIndex(i);
        for (std::size_t j{}; j < DIM; j++)
            binMiddles[i][j] = this->min[j] + (static_cast<double>(idx[j]) + 0.5) * this->step[j];
    }
    return binMiddles;
}

template<std::size_t DIM, typename T>
std::vector<typename Histogram<DIM, T>::BinValue> Histogram<DIM, T>::dumpValues() const {
    std::vector<Vector<DIM>> binMiddles = this->dumpBinMiddles();
    std::vector<BinValue> result;
    result.reserve(this->size());
    std::transform(this->begin(), this->end(), binMiddles.begin(), std::back_inserter(result),
                   [](double binData, const Vector<DIM> &binValue) { return BinValue{binValue, binData}; });
    return result;
}

template<std::size_t DIM, typename T>
std::array<std::size_t, DIM> Histogram<DIM, T>::reshapeIndex(std::size_t flatIdx) const {
    std::array<std::size_t, DIM> idx{};
    for (int i = DIM - 1; i >= 0; i--) {
        idx[i] = flatIdx % this->numBins[i];
        flatIdx /= this->numBins[i];
    }
    return idx;
}

template<std::size_t DIM, typename T>
std::vector<double> Histogram<DIM, T>::getBinDividers(std::size_t idx) const {
    Expects(idx < DIM);

    std::vector<double> result(this->numBins[idx] + 1);
    auto numBinsD = static_cast<double>(this->numBins[idx]);
    for (std::size_t i{}; i <= this->numBins[idx]; i++) {
        auto iD = static_cast<double>(i);
        result[i] = this->min[idx] * ((numBinsD - iD) / numBinsD) + this->max[idx] * (iD / numBinsD);
    }
    return result;
}