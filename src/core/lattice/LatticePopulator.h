//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_LATTICEPOPULATOR_H
#define RAMPACK_LATTICEPOPULATOR_H

#include <vector>

#include "Lattice.h"
#include "core/Shape.h"


/**
 * @brief An interface populating the lattice with a given number of shapes in a specific manner (serial, random, etc.).
 * @details In principle a number of shapes to populate may be smaller than the number of shapes in the lattice. This
 * class is to control which ones to skip.
 */
class LatticePopulator {
public:
    virtual ~LatticePopulator() = default;

    /**
     * @brief Populates a given @a lattice with @a numOfShapes shapes. Which ones are skipped is specific to
     * implementing class.
     */
    [[nodiscard]] virtual std::vector<Shape> populateLattice(const Lattice &lattice, std::size_t numOfShapes) const = 0;
};


#endif //RAMPACK_LATTICEPOPULATOR_H
