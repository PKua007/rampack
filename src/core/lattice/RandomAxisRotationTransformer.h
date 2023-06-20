//
// Created by Piotr Kubala on 20/06/2023.
//

#ifndef RAMPACK_RANDOMAXISROTATIONTRANSFORMER_H
#define RAMPACK_RANDOMAXISROTATIONTRANSFORMER_H


#include <random>
#include <variant>

#include "LatticeTransformer.h"


class RandomAxisRotationTransformer : public LatticeTransformer {
public:
    using Axis = std::variant<Vector<3>, ShapeGeometry::Axis>;

private:
    mutable std::mt19937 mt;
    Axis axis;

    void rotateRandomly(Shape &shape, const ShapeGeometry &geometry) const;

public:
    RandomAxisRotationTransformer(const Axis &axis, unsigned long seed);

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_RANDOMAXISROTATIONTRANSFORMER_H
