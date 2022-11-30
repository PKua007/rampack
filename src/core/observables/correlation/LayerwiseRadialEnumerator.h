//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_LAYERWISERADIALENUMERATOR_H
#define RAMPACK_LAYERWISERADIALENUMERATOR_H

#include <array>

#include "PairEnumerator.h"


/**
 * @brief PairEnumerator, which first associates molecules to layers and then enumerates pairs of molecules lying in the
 * same layer with the distance computed along the layer.
 * @details It supports multi-threaded enumeration if PairConsumer supports it. The layer positions are recognized
 * automatically using smectic order parameter for Miller indices specified in the constructor.
 */
class LayerwiseRadialEnumerator : public PairEnumerator {
private:
    Vector<3> millerIndices{};
    std::string focalPointName{};

    [[nodiscard]] Vector<3> calculateK(const Packing &packing) const;
    [[nodiscard]] static double calculateTauAngle(const Vector<3> &kVector, const std::vector<Vector<3>> &focalPoints);

public:
    /**
     * @brief Creates the enumerator for layers described by Miller indices @a millerIndices. Points named @a
     * focalPointName will be used to calculate layering and computing distances (see ShapeGeometry::getNamedPointForShape).
     */
    explicit LayerwiseRadialEnumerator(const std::array<int, 3> &millerIndices, std::string focalPointName = "o");

    /**
     * @brief Returns expected number of molecules in circular shells between adjacent @a radiiBounds for a given
     * @a packing assuming that molecules are distributed uniformly.
     */
    [[nodiscard]] std::vector<double>
    getExpectedNumOfMoleculesInShells(const Packing &packing, const std::vector<double> &radiiBounds) const override;

    /**
     * @brief Enumerated pairs in a way described in class' description.
     */
    void enumeratePairs(const Packing &packing, const ShapeTraits &traits, PairConsumer &pairConsumer) const override;

    /**
     * @brief Returns signature name "lr" (from "layerwise radial")
     */
    [[nodiscard]] std::string getSignatureName() const override { return "lr"; }
};


#endif //RAMPACK_LAYERWISERADIALENUMERATOR_H
