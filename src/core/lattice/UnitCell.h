//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_UNITCELL_H
#define RAMPACK_UNITCELL_H

#include <memory>
#include <vector>

#include "core/Shape.h"
#include "core/TriclinicBox.h"


class UnitCell {
private:
    std::shared_ptr<TriclinicBox> cellShape;
    std::vector<Shape> molecules;

public:
    using const_iterator = decltype(molecules)::const_iterator;
    using iterator = decltype(molecules)::iterator;

    UnitCell(std::shared_ptr<TriclinicBox> cellShape, std::vector<Shape> molecules);

    UnitCell(const TriclinicBox &cellShape, std::vector<Shape> molecules)
            : UnitCell(std::make_shared<TriclinicBox>(cellShape), std::move(molecules))
    { }

    [[nodiscard]] std::size_t size() const { return this->molecules.size(); }
    [[nodiscard]] Shape &operator[](std::size_t i) { return this->molecules.at(i); }
    [[nodiscard]] const Shape &operator[](std::size_t i) const { return this->molecules.at(i); }
    [[nodiscard]] TriclinicBox &getCellShape() { return *this->cellShape; }
    [[nodiscard]] const TriclinicBox &getCellShape() const { return *this->cellShape; }
    [[nodiscard]] iterator begin() { return this->molecules.begin(); }
    [[nodiscard]] iterator end() { return this->molecules.end(); }
    [[nodiscard]] const_iterator begin() const { return this->molecules.begin(); }
    [[nodiscard]] const_iterator end() const { return this->molecules.end(); }
};


#endif //RAMPACK_UNITCELL_H
