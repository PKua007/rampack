//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_RANDOMPOPULATOR_H
#define RAMPACK_RANDOMPOPULATOR_H

#include <random>

#include "LatticePopulator.h"


class RandomPopulator : public LatticePopulator {
private:
    mutable std::default_random_engine rng;

public:
    explicit RandomPopulator(unsigned long seed) : rng(seed) { }

    [[nodiscard]] std::vector<Shape> populateLattice(const Lattice &lattice, std::size_t numOfShapes) const override;
};


#endif //RAMPACK_RANDOMPOPULATOR_H
