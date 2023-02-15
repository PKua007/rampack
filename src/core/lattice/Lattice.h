//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_LATTICE_H
#define RAMPACK_LATTICE_H

#include <vector>
#include <array>

#include "UnitCell.h"


/**
 * @brief A class representing regular or irregular lattice of UnitCell -s.
 * @details <p> Freshly constructed object is always a regular lattice. Regular lattice is represented by a single
 * UnitCell. All methods with @a xxxUnitCellxxx token in the name refer to this cell. Methods with @a xxxSpecificCellxxx
 * token in the name also work and regardless of cell indices are applied to the same unit cell. The only exception is
 * modifySpecificCellMolecules() method, which turns the lattice into an irregular lattice, where from there on each
 * cell can be altered separately. For irregular lattice, all @a xxxUnitCellxxx methods throw exceptions and only ones
 * with @a xxxSpecificCellxxx can be used.
 *
 * <p> Regardless of the lattice type, getCellBox() and modifyCellBox() methods can be used to access and modify the
 * cell box of all cells simultaneously without changing the lattice type. Likewise, all cell boxes are synchronized and
 * cannot be changed separately. Cell box is not controllable using a @a std::shared_ptr used to created UnitCell passed
 * to the constructor - on construction, the class creates a deep copy of the UnitCell.
 */
class Lattice {
private:
    std::vector<UnitCell> cells;
    std::array<std::size_t, 3> dimensions;
    std::size_t numCells{};
    bool isRegular_{};

    [[nodiscard]] std::size_t getCellIndex(std::size_t i, std::size_t j, std::size_t k) const;
    void normalizeRegular();
    void normalizeIrregular();

public:
    /**
     * @brief Create a regular lattice with a given @a unitCell and number of cells given by @a dimensions.
     * @details A deep copy (see UnitCell::deepCopy()) of @a unitCell is used to desynchronize the lattice from
     * uncontrolled external changes.
     */
    Lattice(const UnitCell &unitCell, const std::array<std::size_t, 3> &dimensions);

    /**
     * @brief Returns a read-only specific cell of given integer indices. For a regular lattice it is always the same
     * cell regardless of the indices.
     */
    [[nodiscard]] const UnitCell &getSpecificCell(std::size_t i, std::size_t j, std::size_t k) const;

    /**
     * @brief Returns a read-only unit cell of the regular lattice. Irregular lattice throws.
     */
    [[nodiscard]] const UnitCell &getUnitCell() const;

    /**
     * @brief Returns a read-only list of molecules in the cell with given integer indices. For a regular lattice it is
     * always the same cell regardless of the indices.
     */
    [[nodiscard]] const std::vector<Shape> &getSpecificCellMolecules(std::size_t i, std::size_t j, std::size_t k) const;

    /**
     * @brief Returns a read-only list of molecules in the unit cell of the regular lattice. Irregular lattice throws.
     */
    [[nodiscard]] const std::vector<Shape> &getUnitCellMolecules() const;

    /**
     * @brief Returns read-only box of the lattice cell (all cells have the same box).
     */
    [[nodiscard]] const TriclinicBox &getCellBox() const { return this->cells.front().getBox(); }

    /**
     * @brief Returns a modifiable list of molecules in the cell with given integer indices. If the lattice was regular
     * prior to this invocation, it is turned into an irregular lattice.
     */
    [[nodiscard]] std::vector<Shape> &modifySpecificCellMolecules(std::size_t i, std::size_t j, std::size_t k);

    /**
     * @brief Returns a modifiable list of molecules in the unit cell of the regular lattice. Irregular lattice throws.
     * @details In contrary to modifySpecificCellMolecules(), this method does not turn the lattice into an irregular
     * one.
     */
    [[nodiscard]] std::vector<Shape> &modifyUnitCellMolecules();

    /**
     * @brief Returns modifiable box of the lattice cell (all cells have the same box). It does not change the lattice
     * type (regular or irregular).
     */
    [[nodiscard]] TriclinicBox &modifyCellBox() { return this->cells.front().getBox(); }

    /**
     * @brief Normalizes the lattice.
     * @details In essence, all molecules in cells are fitted so that their relative cell coordinates are normalized
     * (in the range [0, 1)). For a regular lattice it entails only changing the coordinate in a unit cell, however in
     * an irregular lattice molecules may be moved between different cells if their absolute coordinates do not match
     * with the cell they were in.
     */
    void normalize();

    [[nodiscard]] bool isRegular() const { return this->isRegular_; }

    /**
     * @brief Returns @a true, if all molecules in the lattice have normalized relative cell coordinates (in the range
     * [0, 1)).
     */
    [[nodiscard]] bool isNormalized() const;

    /**
     * @brief Calculates and return the box of a whole lattice.
     */
    [[nodiscard]] TriclinicBox getLatticeBox() const;

    /**
     * @brief Returns the number of particles in the whole lattice.
     */
    [[nodiscard]] std::size_t size() const;

    /**
     * @brief Returns number of cells in the lattice in each direction.
     */
    [[nodiscard]] const std::array<std::size_t, 3> &getDimensions() const { return this->dimensions; }

    /**
     * @brief Generates a list of all molecules in the lattice with absolute coordinates, essentially outputting the
     * lattice.
     * @details The order of molecules is an implementation detail, however not random.
     */
    [[nodiscard]] std::vector<Shape> generateMolecules() const;
};


#endif //RAMPACK_LATTICE_H
