//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRCONSUMER_H
#define RAMPACK_PAIRCONSUMER_H

#include "core/Packing.h"
#include "utils/OMPMacros.h"
#include "core/ShapeTraits.h"


class PairConsumer {
private:
    std::size_t numThreads{};

public:
    explicit PairConsumer(std::size_t numThreads = 1) : numThreads{numThreads == 0 ? _OMP_MAXTHREADS : numThreads} { }
    virtual ~PairConsumer() = default;

    virtual void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                             double distance, const ShapeTraits &shapeTraits) = 0;
    [[nodiscard]] std::size_t getMaxThreads() const { return this->numThreads; }
};


#endif //RAMPACK_PAIRCONSUMER_H
