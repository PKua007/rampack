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
    UnitCell(std::shared_ptr<TriclinicBox> cellShape, std::vector<Shape> molecules);

    std::size_t size() const { return this->molecules.size(); }
    Shape &operator[](std::size_t i) { return this->molecules.at(i); }
    const Shape &operator[](std::size_t i) const { return this->molecules.at(i); }
    TriclinicBox &getCellShape() { return *this->cellShape; }
    const TriclinicBox &getCellShape() const { return *this->cellShape; }
};


#endif //RAMPACK_UNITCELL_H
