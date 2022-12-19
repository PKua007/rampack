//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRCONSUMER_H
#define RAMPACK_PAIRCONSUMER_H

#include "core/Packing.h"
#include "utils/OMPMacros.h"
#include "core/ShapeTraits.h"


/**
 * @brief A class capable of consuming pairs emitted by PairEnumerator.
 */
class PairConsumer {
private:
    std::size_t numThreads{};

public:
    /**
     * @brief Constructs a class declaring the support of at most @a numThreads threads.
     * @details Is @a numThreads is equal 0, @a omp_get_max_threads() theads will be used
     */
    explicit PairConsumer(std::size_t numThreads = 1) : numThreads{numThreads == 0 ? OMP_MAXTHREADS : numThreads} { }
    virtual ~PairConsumer() = default;

    /**
     * @brief This method is called by PairEnumerator for each enumerated pair (possibly from different OpenMP threads).
     * @param packing Packing from which the pairs are enumerated
     * @param idxPair pair of indices of shapes from the @a packing
     * @param distance distance between the shapes calculated in a way specific to given PairEnumerator
     * @param shapeTraits ShapeTraits used in the @a packing
     */
    virtual void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                             double distance, const ShapeTraits &shapeTraits) = 0;

    /**
     * @brief Returns max number of supported OpenMP threads specified in the constructor.
     */
    [[nodiscard]] std::size_t getMaxThreads() const { return this->numThreads; }
};


#endif //RAMPACK_PAIRCONSUMER_H
