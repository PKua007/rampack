//
// Created by pkua on 12.09.22.
//

#include "PairAveragedCorrelation.h"


void PairAveragedCorrelation::addSnapshot(const Packing &packing, [[maybe_unused]] double temperature,
                                          [[maybe_unused]] double pressure,
                                          [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    this->lastTraits = &shapeTraits;
    this->pairEnumerator->enumeratePairs(packing, shapeTraits, *this);
    this->histogram.nextSnapshot();
    this->lastTraits = nullptr;
}

void PairAveragedCorrelation::print(std::ostream &out) const {
    for (auto [x, y] : this->histogram.dumpValues(Histogram::ReductionMethod::AVERAGE))
        out << x << " " << y << std::endl;
}

void PairAveragedCorrelation::consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                                          double distance, [[maybe_unused]] double jacobian)
{
    if (distance > this->histogram.getMax())
        return;

    const auto &shape1 = packing[idxPair.first];
    const auto &shape2 = packing[idxPair.second];
    this->histogram.add(this->correlationFunction->calculate(shape1, shape2, *this->lastTraits), distance);
}
