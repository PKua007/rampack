//
// Created by pkua on 12.09.22.
//

#include "PairAveragedCorrelation.h"


void PairAveragedCorrelation::addSnapshot(const Packing &packing, [[maybe_unused]] double temperature,
                                          [[maybe_unused]] double pressure, const ShapeTraits &shapeTraits)
{
    this->pairEnumerator->enumeratePairs(packing, shapeTraits, *this);
    this->histogram.nextSnapshot();
}

void PairAveragedCorrelation::print(std::ostream &out) const {
    for (auto [x, y] : this->histogram.dumpValues(ReductionMethod::AVERAGE))
        out << x.front() << " " << y << std::endl;
}

void PairAveragedCorrelation::consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                                          const Vector<3> &distanceVector, const ShapeTraits &shapeTraits)
{
    double distance = distanceVector.norm();
    if (distance > this->histogram.getMax())
        return;

    const auto &shape1 = packing[idxPair.first];
    const auto &shape2 = packing[idxPair.second];
    this->histogram.add(distance, this->correlationFunction->calculate(shape1, shape2, distanceVector, shapeTraits));
}
