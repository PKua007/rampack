//
// Created by pkua on 02.02.2022.
//

#include "TriclinicBox.h"

void TriclinicBox::absoluteToRelative(std::vector<Shape> &shapes) const {
    for (auto &shape : shapes)
        shape.setPosition(this->inverseDimensions * shape.getPosition());
}

Vector<3> TriclinicBox::absoluteToRelative(const Vector<3> &pos) const {
    return this->inverseDimensions * pos;
}

void TriclinicBox::relativeToAbsolute(std::vector<Shape> &shapes) const {
    for (auto &shape : shapes)
        shape.setPosition(this->dimensions * shape.getPosition());
}

Vector<3> TriclinicBox::relativeToAbsolute(const Vector<3> &pos) const {
    return this->dimensions * pos;
}

TriclinicBox::TriclinicBox(const std::array<Vector<3>, 3> &dimensions) {
    for (std::size_t i{}; i < 3; i++)
        for (std::size_t j{}; j < 3; j++)
            this->dimensions(i, j) = dimensions[j][i];

    this->inverseDimensions = this->dimensions.inverse();
}
