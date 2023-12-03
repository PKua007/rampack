//
// Created by Piotr Kubala on 15/09/2023.
//

#include <algorithm>

#include "BinAveragedFunction.h"
#include "utils/Exceptions.h"
#include "trackers/DummyTracker.h"
#include "Histogram.h"


void BinAveragedFunction::addSnapshot(const Packing &packing, [[maybe_unused]] double temperature,
                                      [[maybe_unused]] double pressure, const ShapeTraits &shapeTraits)
{
    this->tracker->calculateOrigin(packing, shapeTraits);
    Vector<3> trackedOrigin = this->tracker->getOriginPos();
    if (!this->firstOrigin.has_value())
        this->firstOrigin = trackedOrigin;
    Vector<3> originDelta = trackedOrigin - *this->firstOrigin;

    const auto &box = packing.getBox();
    const auto &bc = packing.getBoundaryConditions();
    #pragma omp parallel for shared(packing, bc, box, histogramBuilder, originDelta, shapeTraits) \
            default(none) num_threads(this->numThreads)
    for (std::size_t i = 0; i < packing.size(); i++) {
        const auto &shape = packing[i];
        Vector<3> pos = shape.getPosition();
        pos -= originDelta;
        pos += bc.getCorrection(pos);
        this->shapeFunction->calculate(shape, shapeTraits);
        auto valuesVector = this->shapeFunction->getValues();
        std::valarray<double> values(valuesVector.data(), valuesVector.size());

        this->histogramBuilder.add(box.absoluteToRelative(pos), values);
    }
    this->histogramBuilder.nextSnapshot();
}

void BinAveragedFunction::print(std::ostream &out) const {
    for (const auto &[binMiddle, values, count] : this->histogramBuilder.dumpValues(ReductionMethod::AVERAGE)) {
        out << binMiddle[0] << " " << binMiddle[1] << " " << binMiddle[2];
        for (double value : values)
            out << " " << value;
        if (this->printCount)
            out << " " << count;
        out << std::endl;
    }
}

void BinAveragedFunction::clear() {
    this->histogramBuilder.clear();
    this->tracker->reset();
    this->firstOrigin = std::nullopt;
}

BinAveragedFunction::BinAveragedFunction(const std::array<std::size_t, 3> &numBins,
                                         std::shared_ptr<ShapeFunction> shapeFunction,
                                         std::shared_ptr<GoldstoneTracker> tracker, bool printCount,
                                         std::size_t numThreads)
        : numBins{normalizeNumBins(numBins)}, tracker{std::move(tracker)},
          histogramBuilder({0, 0, 0}, {1, 1, 1}, this->numBins, numThreads,
                           BinAveragedFunction::makeInitialValarray(*shapeFunction)),
          numThreads{numThreads == 0 ? OMP_MAXTHREADS : numThreads},
          shapeFunction{std::move(shapeFunction)}, printCount{printCount}
{
    if (this->tracker == nullptr)
        this->tracker = std::make_unique<DummyTracker>();
    Expects(std::any_of(numBins.begin(), numBins.end(), [](std::size_t n) { return n > 1; }));
}

std::array<std::size_t, 3> BinAveragedFunction::normalizeNumBins(std::array<std::size_t, 3> array) {
    for (auto &elem : array)
        if (elem == 0)
            elem = 1;
    return array;
}

std::vector<Histogram<3, std::valarray<double>>::BinValue> BinAveragedFunction::dumpValues() const {
    return this->histogramBuilder.dumpValues(ReductionMethod::AVERAGE);
}

std::valarray<double> BinAveragedFunction::makeInitialValarray(const ShapeFunction &shapeFunction) {
    std::size_t numValues = shapeFunction.getValues().size();
    return std::valarray<double>(0.0, numValues);
}
