//
// Created by Piotr Kubala on 20/06/2023.
//

#ifndef RAMPACK_RANDOMROTATIONTRANSFORMER_H
#define RAMPACK_RANDOMROTATIONTRANSFORMER_H

#include <random>

#include "LatticeTransformer.h"


class RandomRotationTransformer : public LatticeTransformer {
private:
    mutable std::mt19937 mt;

    void rotateRandomly(Shape &shape) const;

public:
    explicit RandomRotationTransformer(unsigned long seed) : mt(seed) { }

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_RANDOMROTATIONTRANSFORMER_H
