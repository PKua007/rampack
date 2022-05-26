//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_RANDOMPOPULATOR_H
#define RAMPACK_RANDOMPOPULATOR_H

#include <random>

#include "LatticePopulator.h"

/**
 * @brief Populates lattice randomly, however preserving ordering given by Lattice::generateMolecules().
 */
class RandomPopulator : public LatticePopulator {
private:
    mutable std::default_random_engine rng;

public:
    /**
     * @brief Constructs the object.
     * @param seed seed form RNG selecting molecules to populate.
     */
    explicit RandomPopulator(unsigned long seed) : rng(seed) { }

    /**
     * @brief Populates @a lattice randomly with @a numOfShapes shapes leaving out random places in order to meet
     * requested @a numOfShapes, however preserving ordering given by Lattice::generateMolecules().
     */
    [[nodiscard]] std::vector<Shape> populateLattice(const Lattice &lattice, std::size_t numOfShapes) const override;
};


#endif //RAMPACK_RANDOMPOPULATOR_H
