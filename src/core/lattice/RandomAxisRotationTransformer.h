//
// Created by Piotr Kubala on 20/06/2023.
//

#ifndef RAMPACK_RANDOMAXISROTATIONTRANSFORMER_H
#define RAMPACK_RANDOMAXISROTATIONTRANSFORMER_H


#include <random>

#include "LatticeTransformer.h"


class RandomAxisRotationTransformer : public LatticeTransformer {
private:
    mutable std::mt19937 mt;
    Vector<3> axis;

    void rotateRandomly(Shape &shape) const;

public:
    RandomAxisRotationTransformer(const Vector<3> &axis, unsigned long seed);

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_RANDOMAXISROTATIONTRANSFORMER_H
