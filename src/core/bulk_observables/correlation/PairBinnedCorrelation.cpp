//
// Created by pkua on 12.09.22.
//

#include "PairBinnedCorrelation.h"


void PairBinnedCorrelation::addSnapshot(const Packing &packing, double temperature, double pressure,
                                        const ShapeTraits &shapeTraits)
{
    this->pairEnumerator->enumeratePairs(packing, *this);
    this->histogram.nextSnapshot();
}

void PairBinnedCorrelation::print(std::ostream &out) const {
    for (auto [x, y] : this->histogram.dumpValues(Histogram::ReductionMethod::SUM))
        out << x << " " << y << std::endl;
}

void PairBinnedCorrelation::consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                                        double distance, double jacobian)
{

}
