//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRENUMERATOR_H
#define RAMPACK_PAIRENUMERATOR_H

#include "core/Packing.h"
#include "PairConsumer.h"
#include "core/ShapeTraits.h"


/**
 * @brief A class enumerating pairs of molecules with their distances computed in a specific way (radially, along
 * layers, etc.)
 */
class PairEnumerator {
public:
    virtual ~PairEnumerator() = default;

    /**
     * @brief Enumerates pairs for a given @a packing in a specific way to @a pairConsumer by calling
     * PairConsumer::consumePair() method.
     */
    virtual void enumeratePairs(const Packing &packing, const ShapeTraits &traits,
                                PairConsumer &pairConsumer) const = 0;

    /**
     * @brief Assuming the packing is fully homogeneous, returns expected number of molecules in narrow shells bounded
     * by adjacent @a radiiBounds, according to its internal understanding of the distance.
     * @details As all adjacent pair of @a radiiBounds are taken, the resulting vector is one element smaller than it.
     */
    [[nodiscard]] virtual std::vector<double>
    getExpectedNumOfMoleculesInShells(const Packing &packing, const std::vector<double> &radiiBounds) const = 0;

    /**
     * @brief Returns a (short) name of this PairEnumerator used in filenames and reporting.
     */
    [[nodiscard]] virtual std::string getSignatureName() const = 0;
};


#endif //RAMPACK_PAIRENUMERATOR_H
