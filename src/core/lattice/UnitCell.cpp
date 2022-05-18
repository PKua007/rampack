//
// Created by pkua on 18.05.22.
//

#include "UnitCell.h"
#include "utils/Assertions.h"


UnitCell::UnitCell(std::shared_ptr<TriclinicBox> cellShape, std::vector<Shape> molecules)
        : cellShape{std::move(cellShape)}, molecules{std::move(molecules)}
{
    Expects(this->cellShape != nullptr);
    Expects(!this->molecules.empty());
}
