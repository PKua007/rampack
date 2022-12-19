//
// Created by pkua on 31.10.22.
//

#include <algorithm>

#include "DensityHistogram.h"
#include "utils/Assertions.h"
#include "trackers/DummyTracker.h"


void DensityHistogram::addSnapshot(const Packing &packing, [[maybe_unused]] double temperature,
                                   [[maybe_unused]] double pressure, const ShapeTraits &shapeTraits)
{
    this->tracker->calculateOrigin(packing, shapeTraits);
    Vector<3> trackedOrigin = this->tracker->getOriginPos();
    if (!this->firstOrigin.has_value())
        this->firstOrigin = trackedOrigin;
    Vector<3> originDelta = trackedOrigin - *this->firstOrigin;

    std::size_t totalBins = std::accumulate(this->numBins.begin(), this->numBins.end(), 1ul, std::multiplies<>{});
    const auto &box = packing.getBox();
    const auto &bc = packing.getBoundaryConditions();
    #pragma omp parallel for shared(packing, bc, box, histogramBuilder, originDelta, totalBins) default(none) \
            num_threads(this->numThreads)
    for (std::size_t i = 0; i < packing.size(); i++) {
        Vector<3> pos = packing[i].getPosition();
        pos -= originDelta;
        pos += bc.getCorrection(pos);
        this->histogramBuilder.add(box.absoluteToRelative(pos), static_cast<double>(totalBins)/packing.size());
    }
    this->histogramBuilder.nextSnapshot();
}

void DensityHistogram::print(std::ostream &out) const {
    for (auto [binMiddle, value] : this->histogramBuilder.dumpValues(HistogramBuilder3D::ReductionMethod::SUM))
        out << binMiddle[0] << " " << binMiddle[1] << " " << binMiddle[2] << " " << value << std::endl;
}

void DensityHistogram::clear() {
    this->histogramBuilder.clear();
    this->tracker->reset();
    this->firstOrigin = std::nullopt;
}

DensityHistogram::DensityHistogram(const std::array<std::size_t, 3> &numBins, std::unique_ptr<GoldstoneTracker> tracker,
                                   std::size_t numThreads)
        : numBins{normalizeNumBins(numBins)}, tracker{std::move(tracker)},
          histogramBuilder({0, 0, 0}, {1, 1, 1}, this->numBins, numThreads),
          numThreads{numThreads == 0 ? OMP_MAXTHREADS : numThreads}
{
    if (this->tracker == nullptr)
        this->tracker = std::make_unique<DummyTracker>();
    Expects(std::any_of(numBins.begin(), numBins.end(), [](std::size_t n) { return n > 1; }));
}

std::array<std::size_t, 3> DensityHistogram::normalizeNumBins(std::array<std::size_t, 3> array) {
    for (auto &elem : array)
        if (elem == 0)
            elem = 1;
    return array;
}

std::vector<HistogramBuilder3D::BinValue> DensityHistogram::dumpValues() const {
    return this->histogramBuilder.dumpValues(HistogramBuilder3D::ReductionMethod::SUM);
}
