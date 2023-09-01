//
// Created by Piotr Kubala on 31/08/2023.
//

#ifndef RAMPACK_LINEARENUMERATOR_H
#define RAMPACK_LINEARENUMERATOR_H

#include "PairEnumerator.h"


/**
 * @brief PairEnumerator, which groups molecules along one height of the simulation box and calculates distances along
 * this height.
 * @details It supports multi-threaded enumeration if PairConsumer supports it.
 */
class LinearEnumerator : public PairEnumerator {
public:
    /**
     * @brief Axis along a given height of the simulation box.
     */
    enum class Axis : std::size_t {
        /** @brief 1st height: \f$ b_2 \times b_3 \times \f$, where /f$ b_i /f$ are box vectors  */
        X,

        /** @brief 2nd height: \f$ b_3 \times b_1 \times \f$, where /f$ b_i /f$ are box vectors  */
        Y,

        /** @brief 3rd height: \f$ b_1 \times b_2 \times \f$, where /f$ b_i /f$ are box vectors  */
        Z
    };

private:
    Axis axis{};
    std::string focalPointName;

public:
    /**
     * @brief Creates the enumerator.
     * @details @a axis determines along which box height the shapes will be grouped. Points named @a focalPointName wil
     * be used to calculate layering and computing distances (see ShapeGeometry::getNamedPointForShape).
     */
    explicit LinearEnumerator(Axis axis, std::string focalPoint = "o")
            : axis{axis}, focalPointName{std::move(focalPoint)}
    { }

    /**
     * @brief Returns expected number of molecules in box cross sections along a selected axis (see constructor)
     * between adjacent @a radiiBounds for a given @a packing assuming that molecules are distributed uniformly.
     */
    [[nodiscard]] std::vector<double>
    getExpectedNumOfMoleculesInShells(const Packing &packing, const std::vector<double> &radiiBounds) const override;

    /**
     * @brief Enumerated pairs in a way described in class' description.
     */
    void enumeratePairs(const Packing &packing, const ShapeTraits &shapeTraits,
                        PairConsumer &pairConsumer) const override;

    /**
     * @brief Returns signature name "x", "y" or "z" depending on chosen axis (see constructor).
     */
    [[nodiscard]] std::string getSignatureName() const override;
};


#endif //RAMPACK_LINEARENUMERATOR_H
