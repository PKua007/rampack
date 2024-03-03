//
// Created by Piotr Kubala on 03/03/2024.
//

#ifndef RAMPACK_SHAPEPARAMETERRANDOMIZINGTRANSFORMER_H
#define RAMPACK_SHAPEPARAMETERRANDOMIZINGTRANSFORMER_H

#include <memory>
#include <utility>

#include "LatticeTransformer.h"
#include "ShapeParameterRandomizer.h"


class ShapeParameterRandomizingTransformer : public LatticeTransformer {
private:
    void doTransform(Shape &shape, const ShapeTraits &traits) const;

    std::string paramName;
    std::shared_ptr<ShapeParameterRandomizer> randomizer;
    mutable std::mt19937 mt;

public:
    ShapeParameterRandomizingTransformer(std::string paramName,
                                         const std::shared_ptr<ShapeParameterRandomizer> &randomizer,
                                         std::mt19937::result_type seed);

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_SHAPEPARAMETERRANDOMIZINGTRANSFORMER_H
