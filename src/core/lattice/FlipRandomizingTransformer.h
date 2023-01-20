//
// Created by pkua on 20.05.22.
//

#ifndef RAMPACK_FLIPRANDOMIZINGTRANSFORMER_H
#define RAMPACK_FLIPRANDOMIZINGTRANSFORMER_H

#include <random>

#include "LatticeTransformer.h"
#include "core/ShapeGeometry.h"


/**
 * @brief LatticeTransformer performing random flips of all molecules in the lattice.
 * @details It is done by rotating them around their secondary axes placed in their geometric origins (see
 * ShapeGeometry::getSecondaryAxis() and ShapeGeometry::getGeometricOrigin(), respectively). The transformer supports
 * both regular and irregular lattices.
 */
class FlipRandomizingTransformer : public LatticeTransformer {
private:
    mutable std::default_random_engine rng;

public:
    /**
     * @brief Construct the object.
     * @param seed seed of the RNG to sample flips
     */
    explicit FlipRandomizingTransformer(unsigned long seed) : rng(seed) { }

    /**
     * @brief Performs flips.
     * @param lattice lattice, whose molecules should be flipped. All types of lattices are supported, however resulting
     * lattice is always irregular. Lattice does not have to be normalized. If it was normalized prior to flip
     * transformations, it will be renormalized afterwards (as rotation around the geometric origin may require applying
     * a translation, some particles may end up outside their cells).
     */
    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_FLIPRANDOMIZINGTRANSFORMER_H
