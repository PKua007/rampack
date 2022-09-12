//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRCONSUMER_H
#define RAMPACK_PAIRCONSUMER_H

#include "core/Packing.h"


class PairConsumer {
public:
    virtual ~PairConsumer() = default;

    virtual void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                             double distance, double jacobian) = 0;
};


#endif //RAMPACK_PAIRCONSUMER_H
