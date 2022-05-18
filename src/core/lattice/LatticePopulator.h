//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_LATTICEPOPULATOR_H
#define RAMPACK_LATTICEPOPULATOR_H

#include <vector>

#include "Lattice.h"
#include "core/Shape.h"


class LatticePopulator {
public:
    virtual ~LatticePopulator() = default;

    [[nodiscard]] virtual std::vector<Shape> populateLattice(const Lattice &lattice, std::size_t numOfShapes) const = 0;
};


#endif //RAMPACK_LATTICEPOPULATOR_H
