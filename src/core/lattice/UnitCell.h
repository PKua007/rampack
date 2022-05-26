//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_UNITCELL_H
#define RAMPACK_UNITCELL_H

#include <memory>
#include <vector>

#include "core/Shape.h"
#include "core/TriclinicBox.h"


/**
 * @brief A class representing a unit cell of a regular lattice, or a single cell of an irregular lattice.
 * @details TriclinicBox describing the shape of UnitCell is stored as a @a std::shared_ptr so that the copies of cell
 * share the same instance and their shapes are synchronized.
 */
class UnitCell {
private:
    std::shared_ptr<TriclinicBox> cellShape;
    std::vector<Shape> molecules;

public:
    using const_iterator = decltype(molecules)::const_iterator;
    using iterator = decltype(molecules)::iterator;

    /**
     * @brief Constructs the cell with given molecules and a @a std::shared_ptr form of TriclinicBox.
     * @details Shapes positions are represented as relative coordinates in the box.
     */
    UnitCell(std::shared_ptr<TriclinicBox> cellShape, std::vector<Shape> molecules);

    /**
     * @brief Constructs the cell with given molecules and a TriclinicBox, which is copied to a @a std::shared_ptr
     * instance.
     * @details Shapes positions are represented as relative coordinates in the box.
     */
    UnitCell(const TriclinicBox &cellShape, std::vector<Shape> molecules)
            : UnitCell(std::make_shared<TriclinicBox>(cellShape), std::move(molecules))
    { }

    [[nodiscard]] std::size_t size() const { return this->molecules.size(); }
    [[nodiscard]] Shape &operator[](std::size_t i) { return this->molecules.at(i); }
    [[nodiscard]] const Shape &operator[](std::size_t i) const { return this->molecules.at(i); }
    [[nodiscard]] std::vector<Shape> &getMolecules() { return this->molecules; }
    [[nodiscard]] const std::vector<Shape> &getMolecules() const { return this->molecules; }
    [[nodiscard]] TriclinicBox &getBox() { return *this->cellShape; }
    [[nodiscard]] const TriclinicBox &getBox() const { return *this->cellShape; }
    [[nodiscard]] iterator begin() { return this->molecules.begin(); }
    [[nodiscard]] iterator end() { return this->molecules.end(); }
    [[nodiscard]] const_iterator begin() const { return this->molecules.begin(); }
    [[nodiscard]] const_iterator end() const { return this->molecules.end(); }

    /**
     * @brief Returns @a true, if all coordinates of all shapes are in the range [0, 1).
     */
    [[nodiscard]] bool isNormalized() const;

    /**
     * @brief Clones the cell with a new @a std::shared_ptr of the cell box.
     */
    [[nodiscard]] UnitCell deepCopy() const;
};


#endif //RAMPACK_UNITCELL_H
