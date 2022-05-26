//
// Created by pkua on 18.05.22.
//

#include "UnitCell.h"
#include "utils/Assertions.h"


UnitCell::UnitCell(std::shared_ptr<TriclinicBox> cellShape, std::vector<Shape> molecules)
        : cellShape{std::move(cellShape)}, molecules{std::move(molecules)}
{
    Expects(this->cellShape != nullptr);
}

bool UnitCell::isNormalized() const {
    for (const auto &shape : this->molecules)
        for (double coord : shape.getPosition())
            if (coord < 0 || coord >= 1)
                return false;
    return true;
}

UnitCell UnitCell::deepCopy() const {
    return UnitCell(std::make_shared<TriclinicBox>(*this->cellShape), this->molecules);
}
