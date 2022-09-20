//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_RADIALENUMERATOR_H
#define RAMPACK_RADIALENUMERATOR_H

#include "PairEnumerator.h"


class RadialEnumerator : public PairEnumerator {
private:
    std::string focalPointName;

public:
    explicit RadialEnumerator(std::string focalPoint = "cm") : focalPointName{std::move(focalPoint)} { }

    [[nodiscard]] std::vector<double>
    getExpectedNumOfMoleculesInShells(const Packing &packing, const std::vector<double> &radiiBounds) const override;

    void enumeratePairs(const Packing &packing, const ShapeTraits &shapeTraits,
                        PairConsumer &pairConsumer) const override;
    [[nodiscard]] std::string getSignatureName() const override { return "r"; }
};


#endif //RAMPACK_RADIALENUMERATOR_H
