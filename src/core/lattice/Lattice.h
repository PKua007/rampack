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
    std::size_t numCells{};
    bool isRegular_{};

    std::size_t getCellIndex(std::size_t i, std::size_t j, std::size_t k) const;

public:
    Lattice(const UnitCell &unitCell, const std::array<std::size_t, 3> &dimensions);

    [[nodiscard]] const UnitCell &getSpecificCell(std::size_t i, std::size_t j, std::size_t k) const;
    [[nodiscard]] const UnitCell &getUnitCell() const;
    [[nodiscard]] const std::vector<Shape> &getSpecificCellMolecules(std::size_t i, std::size_t j, std::size_t k) const;
    [[nodiscard]] const std::vector<Shape> &getUnitCellMolecules() const;
    [[nodiscard]] const TriclinicBox &getCellBox() const { return this->cells.front().getBox(); }

    [[nodiscard]] std::vector<Shape> &modifySpecificCellMolecules(std::size_t i, std::size_t j, std::size_t k);
    [[nodiscard]] std::vector<Shape> &modifyUnitCellMolecules();
    [[nodiscard]] TriclinicBox &modifyCellBox() { return this->cells.front().getBox(); }

    [[nodiscard]] bool isRegular() const { return this->isRegular_; }
    [[nodiscard]] TriclinicBox getLatticeBox() const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] const std::array<std::size_t, 3> &getDimensions() const { return this->dimensions; }
    [[nodiscard]] std::vector<Shape> generateMolecules() const;
};


#endif //RAMPACK_LATTICE_H
