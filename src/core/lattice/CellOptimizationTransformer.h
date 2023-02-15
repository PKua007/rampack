//
// Created by pkua on 20.05.22.
//

#ifndef RAMPACK_CELLOPTIMIZATIONTRANSFORMER_H
#define RAMPACK_CELLOPTIMIZATIONTRANSFORMER_H

#include "LatticeTransformer.h"
#include "core/Interaction.h"


/**
 * @brief Lattice transformer which optimizes the size of the cell without introducing overlaps and not changing the
 * angles. Works for both regular and irregular lattices.
 */
class CellOptimizationTransformer : public LatticeTransformer {
private:
    const std::string axisOrderString;
    std::array<double, 3> spacings{};

public:
    /**
     * @brief Creates the object with given parameters.
     * @param axisOrderString the order in which the axis of the cell should be optimized
     * (see DistanceOptimizer::shrinkPacking)
     * @param spacing the distance between cell faces to be introduced
     */
    CellOptimizationTransformer(const std::string &axisOrderString, double spacing)
            : CellOptimizationTransformer(axisOrderString, {spacing, spacing, spacing})
    { }

    /**
     * @brief Same as CellOptimizationTransformer(const std::string&, double), but each cell axis
     * has a separate spacing given by the corresponding elements of @a spacings.
     */
    CellOptimizationTransformer(const std::string &axisOrderString, const std::array<double, 3> &spacings);

    /**
     * @brief Performs cell size optimization for a given @a lattice.
     * @details First, using DistanceOptimizer, it finds the smallest possible cell size which does not introduce
     * overlaps in an order given by @a axisOrderString from the constructor. After is has been found, distance between
     * cell faces is increased by @a spacing (in the direction ortohgonal to the face).
     * @param lattice lattice to optimized. It can be either regular or irregular, but is has to be normalized (see
     * Lattice::normalize())
     * @param shapeTraits ShapeTraits of the shape residing in the lattice.
     */
    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_CELLOPTIMIZATIONTRANSFORMER_H
