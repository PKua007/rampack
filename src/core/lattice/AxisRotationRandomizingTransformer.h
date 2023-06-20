//
// Created by Piotr Kubala on 20/06/2023.
//

#ifndef RAMPACK_AXISROTATIONRANDOMIZINGTRANSFORMER_H
#define RAMPACK_AXISROTATIONRANDOMIZINGTRANSFORMER_H


#include <random>
#include <variant>

#include "LatticeTransformer.h"


/**
 * @brief Lattice transformer which performs random rotations of particles around the selected axis.
 */
class AxisRotationRandomizingTransformer : public LatticeTransformer {
public:
    /**
     * @brief Axis variant: extrinsic axis (Vector<3>) or intrinsic axis (ShapeGeometry::Axis)
     */
    using Axis = std::variant<Vector<3>, ShapeGeometry::Axis>;

private:
    mutable std::mt19937 mt;
    Axis axis;

    void rotateRandomly(Shape &shape, const ShapeGeometry &geometry) const;

public:
    /**
     * @brief Setups the class for axis @a axis with RNG seed @a seed.
     */
    AxisRotationRandomizingTransformer(const Axis &axis, unsigned long seed);

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_AXISROTATIONRANDOMIZINGTRANSFORMER_H
