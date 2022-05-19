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

    [[nodiscard]] const UnitCell &getCell(std::size_t i, std::size_t j, std::size_t k) const;
    UnitCell &modifyCell(std::size_t i, std::size_t j, std::size_t k);
    [[nodiscard]] bool isRegular() const { return this->isRegular_; }
    [[nodiscard]] TriclinicBox &getCellShape() { return this->cells.front().getCellShape(); }
    [[nodiscard]] const TriclinicBox &getCellShape() const { return this->cells.front().getCellShape(); }
    [[nodiscard]] TriclinicBox getLatticeBox() const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] const std::array<std::size_t, 3> &getDimensions() const { return this->dimensions; }
    [[nodiscard]] std::vector<Shape> generateShapes() const;
};


#endif //RAMPACK_LATTICE_H
