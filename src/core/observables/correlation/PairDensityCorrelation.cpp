//
// Created by pkua on 12.09.22.
//

#include "PairDensityCorrelation.h"


void PairDensityCorrelation::addSnapshot(const Packing &packing, [[maybe_unused]] double temperature,
                                         [[maybe_unused]] double pressure, const ShapeTraits &shapeTraits)
{
    this->pairEnumerator->enumeratePairs(packing, shapeTraits, *this);
    auto binDividers = this->histogram.getBinDividers();
    auto factors = this->pairEnumerator->getExpectedNumOfMoleculesInShells(packing, binDividers);
    for (auto &factor : factors)
        factor = 2. / factor / static_cast<double>(packing.size());
    this->histogram.renormalizeBins(factors);
    this->histogram.nextSnapshot();
}

void PairDensityCorrelation::print(std::ostream &out) const {
    for (auto [x, y, count] : this->histogram.dumpValues(ReductionMethod::SUM)) {
        out << x.front() << " " << y;
        if (this->printCount)
            out << " " << count;
        out << std::endl;
    }
}

void PairDensityCorrelation::consumePair([[maybe_unused]] const Packing &packing,
                                         const std::pair<std::size_t, std::size_t> &idxPair,
                                         const Vector<3> &distanceVector,
                                         [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    if (idxPair.first == idxPair.second)
        return;
    double distance = distanceVector.norm();
    if (distance > this->histogram.getMax())
        return;

    this->histogram.add(distance, 1);
}
