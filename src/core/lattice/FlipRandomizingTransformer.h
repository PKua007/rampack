//
// Created by pkua on 20.05.22.
//

#ifndef RAMPACK_FLIPRANDOMIZINGTRANSFORMER_H
#define RAMPACK_FLIPRANDOMIZINGTRANSFORMER_H

#include <random>

#include "LatticeTransformer.h"
#include "core/ShapeGeometry.h"


class FlipRandomizingTransformer : public LatticeTransformer {
private:
    const ShapeGeometry &geometry;
    mutable std::default_random_engine rng;

public:
    FlipRandomizingTransformer(const ShapeGeometry &geometry, unsigned long seed) : geometry{geometry}, rng(seed) { }

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_FLIPRANDOMIZINGTRANSFORMER_H
