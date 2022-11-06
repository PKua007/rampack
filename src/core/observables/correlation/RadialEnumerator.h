//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_RADIALENUMERATOR_H
#define RAMPACK_RADIALENUMERATOR_H

#include "PairEnumerator.h"


/**
 * @brief PairEnumerator yielding pairs with their standard, Euclidean distances.
 * @details It supports multi-threaded enumeration if PairConsumer supports it.
 */
class RadialEnumerator : public PairEnumerator {
private:
    std::string focalPointName;

public:
    /**
     * @brief Constructs the class. Points named @a focalPoint will be used to calculate distances (see
     * ShapeGeometry::getNamedPointForShape).
     */
    explicit RadialEnumerator(std::string focalPoint = "cm") : focalPointName{std::move(focalPoint)} { }

    /**
     * @brief Returns expected number of molecules in the @a packing in thin spherical shells between adjacent radii in
     * @a radiiBounds.
     */
    [[nodiscard]] std::vector<double>
    getExpectedNumOfMoleculesInShells(const Packing &packing, const std::vector<double> &radiiBounds) const override;

    void enumeratePairs(const Packing &packing, const ShapeTraits &shapeTraits,
                        PairConsumer &pairConsumer) const override;

    /**
     * Returns "r" (from "radial") as the signature name.
     */
    [[nodiscard]] std::string getSignatureName() const override { return "r"; }
};


#endif //RAMPACK_RADIALENUMERATOR_H
