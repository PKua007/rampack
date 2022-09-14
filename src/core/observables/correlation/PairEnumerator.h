//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRENUMERATOR_H
#define RAMPACK_PAIRENUMERATOR_H

#include "core/Packing.h"
#include "PairConsumer.h"
#include "core/ShapeTraits.h"


class PairEnumerator {
public:
    virtual ~PairEnumerator() = default;

    virtual void enumeratePairs(const Packing &packing, const ShapeTraits &traits,
                                PairConsumer &pairConsumer) const = 0;

    [[nodiscard]] virtual std::vector<double>
    getExpectedNumOfMoleculesInShells(const Packing &packing, const std::vector<double> &radiiBounds) const = 0;

    [[nodiscard]] virtual std::string getSignatureName() const = 0;
};


#endif //RAMPACK_PAIRENUMERATOR_H
