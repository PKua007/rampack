//
// Created by Piotr Kubala on 03/03/2024.
//

#ifndef RAMPACK_SHAPEPARAMETERRANDOMIZINGTRANSFORMER_H
#define RAMPACK_SHAPEPARAMETERRANDOMIZINGTRANSFORMER_H

#include <memory>
#include <utility>

#include "core/lattice/LatticeTransformer.h"
#include "core/lattice/ShapeParameterRandomizer.h"


/**
 * @brief LatticeTransformer which randomizes ShapeData of shapes in the lattice.
 */
class ShapeParameterRandomizingTransformer : public LatticeTransformer {
private:
    void doTransform(Shape &shape, const ShapeTraits &traits) const;

    std::string paramName;
    std::shared_ptr<ShapeParameterRandomizer> randomizer;
    mutable std::mt19937 mt;

public:
    /**
     * @brief Creates the transformer. The caller must ensure that the concrete randomizer @a randomizer is compatible
     * with the shape param named @a paramName.
     * @param paramName name of the parameter in serialized @ref TextualShapeData to be randomized
     * @param randomizer concrete way of randomizing the parameter
     * @param seed seed for @a randomizer
     */
    ShapeParameterRandomizingTransformer(std::string paramName,
                                         const std::shared_ptr<ShapeParameterRandomizer> &randomizer,
                                         std::mt19937::result_type seed);

    /**
     * @brief Performs the randomization of Lattice @a lattice.
     * @details It is done by performing the following steps:
     * 1. serializing ShapeData of each shape (ShapeDataManager::serialize)
     * 2. applying the method ShapeParameterRandomizer::randomize of constructor's @a randomizer argument on the
     * @ref TextualShapeData field named as given by constructor's @a paramName argument
     * 3. deserializing back the @ref TextualShapeData into ShapeData (ShapeDataManager::deserialize)
     *
     * @throws TransformerException if @a paramName from the constructor in not a valid key of deserialized
     * @ref TextualShapeData, or if ShapeDataManager::deserialize throws
     */
    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_SHAPEPARAMETERRANDOMIZINGTRANSFORMER_H
