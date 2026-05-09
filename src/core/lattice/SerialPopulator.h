//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_SERIALPOPULATOR_H
#define RAMPACK_SERIALPOPULATOR_H

#include "LatticePopulator.h"
#include "LatticeTraits.h"
#include "utils/Utils.h"

/**
 * @brief Populates the lattice in a serial way filling the cell in a given axis order with @a numOfShapes shapes.
 */
class SerialPopulator : public LatticePopulator {
private:
    std::array<std::size_t, 3> axisOrder{};
    std::size_t startFrom{};

public:
    /**
     * @brief Constructs the object.
     * @param axisOrderString order in which axes will be looped through.
     * @param startFrom skip that many shapes and start filling from the next
     */
    explicit SerialPopulator(const std::string &axisOrderString, const std::size_t startFrom = 0)
            : axisOrder{LatticeTraits::parseAxisOrder(axisOrderString)}, startFrom{startFrom}
    { }

    /**
     * @brief Returns molecules for a given @a lattice populating it cell by cell and looping through them in the axis
     * order given in the constructor until requested @a numOfShapes is reached.
     * @details If axis order is, for example, YZX, the innermost loop is through X lattice cell index, so first, X
     * column is filled, then next columns forming XZ plane (Z is the middle loop), and then those XZ planes fill the
     * lattice (Y is the outermost loop). All cells but last are filled fully (the last one may not be filled fully in
     * order to meet @a numOfShapes target). Order of shapes within the cell is preserved.
     */
    [[nodiscard]] std::vector<Shape> populateLattice(const Lattice &lattice, std::size_t numOfShapes) const override;
};


#endif //RAMPACK_SERIALPOPULATOR_H
