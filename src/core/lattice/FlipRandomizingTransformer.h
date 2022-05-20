//
// Created by pkua on 20.05.22.
//

#ifndef RAMPACK_FLIPRANDOMIZINGTRANSFORMER_H
#define RAMPACK_FLIPRANDOMIZINGTRANSFORMER_H

#include <random>

#include "LatticeTransformer.h"


class FlipRandomizingTransformer : public LatticeTransformer {
private:
    Vector<3> secondaryAxis{};
    mutable std::default_random_engine rng;

public:
    FlipRandomizingTransformer(const Vector<3> &secondaryAxis, unsigned long seed);

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_FLIPRANDOMIZINGTRANSFORMER_H
