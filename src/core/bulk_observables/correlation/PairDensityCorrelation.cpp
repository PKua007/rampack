//
// Created by pkua on 12.09.22.
//

#include "PairDensityCorrelation.h"


void PairDensityCorrelation::addSnapshot(const Packing &packing, double temperature, double pressure,
                                         const ShapeTraits &shapeTraits)
{
    this->pairEnumerator->enumeratePairs(packing, *this);
    this->histogram.nextSnapshot();
}

void PairDensityCorrelation::print(std::ostream &out) const {

}

void PairDensityCorrelation::consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                                         double distance, double jacobian)
{

}
