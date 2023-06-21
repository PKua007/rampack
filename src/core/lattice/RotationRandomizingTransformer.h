//
// Created by Piotr Kubala on 20/06/2023.
//

#ifndef RAMPACK_ROTATIONRANDOMIZINGTRANSFORMER_H
#define RAMPACK_ROTATIONRANDOMIZINGTRANSFORMER_H


#include <random>
#include <variant>

#include "LatticeTransformer.h"


/**
 * @brief Lattice transformer which performs random rotations of particles around the selected axis.
 */
class RotationRandomizingTransformer : public LatticeTransformer {
public:
    /**
     * @brief Type for random axis tag.
     */
    struct RandomAxisType {};

    /**
     * @brief Axis variant: extrinsic axis (Vector<3>), intrinsic axis (ShapeGeometry::Axis) or random axis
     * (RotationRandomizingTransformer::RANDOM_AXIS).
     */
    using Axis = std::variant<Vector<3>, ShapeGeometry::Axis, RandomAxisType>;

    /**
     * @brief Random axis tag.
     */
    static constexpr RandomAxisType RANDOM_AXIS{};

private:
    mutable std::mt19937 mt;
    Axis axis;

    void rotateRandomly(Shape &shape, const ShapeGeometry &geometry) const;

public:
    /**
     * @brief Setups the class for axis @a axis with RNG seed @a seed.
     */
    RotationRandomizingTransformer(const Axis &axis, unsigned long seed);

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_ROTATIONRANDOMIZINGTRANSFORMER_H
