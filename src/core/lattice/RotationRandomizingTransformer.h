//
// Created by Piotr Kubala on 20/06/2023.
//

#ifndef RAMPACK_ROTATIONRANDOMIZINGTRANSFORMER_H
#define RAMPACK_ROTATIONRANDOMIZINGTRANSFORMER_H

#include <random>

#include "LatticeTransformer.h"


class RotationRandomizingTransformer : public LatticeTransformer {
private:
    mutable std::mt19937 mt;

    void rotateRandomly(Shape &shape) const;

public:
    explicit RotationRandomizingTransformer(unsigned long seed) : mt(seed) { }

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_ROTATIONRANDOMIZINGTRANSFORMER_H
