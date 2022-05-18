//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_LATTICE_H
#define RAMPACK_LATTICE_H

#include <vector>
#include <array>

#include "UnitCell.h"


class Lattice {
private:
    std::vector<UnitCell> cells;
    std::array<std::size_t, 3> dimensions;
    bool isRegular_{};

    std::size_t getCellIndex(std::size_t i, std::size_t j, std::size_t k) const;

public:
    Lattice(const UnitCell &unitCell, const std::array<std::size_t, 3> &dimensions);
    Lattice(std::vector<UnitCell> unitCells, const std::array<std::size_t, 3> &dimensions);

    const UnitCell &getCell(std::size_t i, std::size_t j, std::size_t k) const;
    UnitCell &modifyCell(std::size_t i, std::size_t j, std::size_t k);
    bool isRegular() const { return this->isRegular_; }
};


#endif //RAMPACK_LATTICE_H
