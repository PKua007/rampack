//
// Created by Piotr Kubala on 31/08/2023.
//

#ifndef RAMPACK_PAIRCOLLECTOR_H
#define RAMPACK_PAIRCOLLECTOR_H

#include <map>

#include "core/observables/correlation/PairConsumer.h"


class PairCollector : public PairConsumer {
public:
    using PairMap = std::map<std::pair<std::size_t, std::size_t>, double>;

    PairMap pairData;

    void consumePair([[maybe_unused]] const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                     double distance, [[maybe_unused]] const ShapeTraits &shapeTraits) override
    {
        CHECK(this->pairData.find(idxPair) == this->pairData.end());
        this->pairData[idxPair] = distance;
    }
};


#endif //RAMPACK_PAIRCOLLECTOR_H
