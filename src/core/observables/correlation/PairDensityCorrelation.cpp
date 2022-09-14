//
// Created by pkua on 12.09.22.
//

#include "PairDensityCorrelation.h"


void PairDensityCorrelation::addSnapshot(const Packing &packing, [[maybe_unused]] double temperature,
                                         [[maybe_unused]] double pressure, const ShapeTraits &shapeTraits)
{
    this->pairEnumerator->enumeratePairs(packing, shapeTraits, *this);
    this->histogram.nextSnapshot();
}

void PairDensityCorrelation::print(std::ostream &out) const {
    for (auto [x, y] : this->histogram.dumpValues(Histogram::ReductionMethod::SUM))
        out << x << " " << y << std::endl;
}

void PairDensityCorrelation::consumePair([[maybe_unused]] const Packing &packing,
                                         [[maybe_unused]] const std::pair<std::size_t, std::size_t> &idxPair,
                                         double distance, double jacobian)
{
    if (jacobian == 0)
        return;
    if (distance > this->histogram.getMax())
        return;

    this->histogram.add(distance, 1. / (jacobian * this->histogram.getBinSize()));
}
