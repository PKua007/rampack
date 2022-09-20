//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_LAYERWISERADIALENUMERATOR_H
#define RAMPACK_LAYERWISERADIALENUMERATOR_H

#include <array>

#include "PairEnumerator.h"


class LayerwiseRadialEnumerator : public PairEnumerator {
private:
    Vector<3> millerIndices{};
    std::string focalPointName{};

    [[nodiscard]] Vector<3> calculateK(const Packing &packing) const;
    [[nodiscard]] static double calculateTauAngle(const Vector<3> &kVector, const std::vector<Vector<3>> &focalPoints);

public:
    explicit LayerwiseRadialEnumerator(const std::array<int, 3> &millerIndices, std::string focalPointName = "cm");

    [[nodiscard]] std::vector<double>
    getExpectedNumOfMoleculesInShells(const Packing &packing, const std::vector<double> &radiiBounds) const override;

    void enumeratePairs(const Packing &packing, const ShapeTraits &traits, PairConsumer &pairConsumer) const override;
    [[nodiscard]] std::string getSignatureName() const override { return "lr"; }
};


#endif //RAMPACK_LAYERWISERADIALENUMERATOR_H
