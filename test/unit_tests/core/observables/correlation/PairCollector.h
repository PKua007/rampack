//
// Created by Piotr Kubala on 31/08/2023.
//

#ifndef RAMPACK_PAIRCOLLECTOR_H
#define RAMPACK_PAIRCOLLECTOR_H

#include <map>

#include "core/observables/correlation/PairConsumer.h"


class PairCollector : public PairConsumer {
public:
    using PairMap = std::map<std::pair<std::size_t, std::size_t>, Vector<3>>;

    PairMap pairData;

    void consumePair([[maybe_unused]] const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                     const Vector<3> &distanceVector, [[maybe_unused]] const ShapeTraits &shapeTraits) override
    {
        CHECK(this->pairData.find(idxPair) == this->pairData.end());
        this->pairData[idxPair] = distanceVector;
    }
};


#endif //RAMPACK_PAIRCOLLECTOR_H
