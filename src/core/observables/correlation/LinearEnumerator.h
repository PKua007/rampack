//
// Created by Piotr Kubala on 31/08/2023.
//

#ifndef RAMPACK_LINEARENUMERATOR_H
#define RAMPACK_LINEARENUMERATOR_H

#include "PairEnumerator.h"


class LinearEnumerator : public PairEnumerator {
public:
    enum class Axis : std::size_t {
        X,
        Y,
        Z
    };

private:
    Axis axis{};
    std::string focalPointName;

public:
    explicit LinearEnumerator(Axis axis, std::string focalPoint = "o")
            : axis{axis}, focalPointName{std::move(focalPoint)}
    { }

    [[nodiscard]] std::vector<double>
    getExpectedNumOfMoleculesInShells(const Packing &packing, const std::vector<double> &radiiBounds) const override;

    void enumeratePairs(const Packing &packing, const ShapeTraits &shapeTraits,
                        PairConsumer &pairConsumer) const override;

    [[nodiscard]] std::string getSignatureName() const override;
};


#endif //RAMPACK_LINEARENUMERATOR_H
