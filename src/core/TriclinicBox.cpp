//
// Created by pkua on 02.02.2022.
//

#include "TriclinicBox.h"

void TriclinicBox::absoluteToRelative(std::vector<Shape> &shapes) const {
    for (auto &shape : shapes)
        shape.setPosition(this->inverseDimensions * shape.getPosition());
}

void TriclinicBox::relativeToAbsolute(std::vector<Shape> &shapes) const {
    for (auto &shape : shapes)
        shape.setPosition(this->dimensions * shape.getPosition());
}

TriclinicBox::TriclinicBox(const std::array<Vector<3>, 3> &dimensions) {
    for (std::size_t i{}; i < 3; i++)
        for (std::size_t j{}; j < 3; j++)
            this->dimensions(i, j) = dimensions[j][i];

    this->inverseDimensions = this->dimensions.inverse();
}

void TriclinicBox::transform(const Matrix<3, 3> &transformation) {
    this->dimensions = transformation * dimensions;
    this->inverseDimensions = this->dimensions.inverse();
}

void TriclinicBox::scale(const std::array<double, 3> &factors) {
    this->transform(Matrix<3, 3>{factors[0], 0, 0,
                                 0, factors[1], 0,
                                 0, 0, factors[2]});
}

std::array<Vector<3>, 3> TriclinicBox::getSides() const {
    Vector<3> side1 = this->relativeToAbsolute(Vector<3>{1, 0, 0});
    Vector<3> side2 = this->relativeToAbsolute(Vector<3>{0, 1, 0});
    Vector<3> side3 = this->relativeToAbsolute(Vector<3>{0, 0, 1});

    return {side1, side2, side3};
}

double TriclinicBox::getVolume() const {
    auto sides = this->getSides();
    return std::abs((sides[0] ^ sides[1]) * sides[2]);
}

std::array<double, 3> TriclinicBox::getHeights() const {
    Vector<3> side1 = this->relativeToAbsolute(Vector<3>{1, 0, 0});
    Vector<3> side2 = this->relativeToAbsolute(Vector<3>{0, 1, 0});
    Vector<3> side3 = this->relativeToAbsolute(Vector<3>{0, 0, 1});
    double vol = this->getVolume();

    double h1 = vol / (side2 ^ side3).norm();
    double h2 = vol / (side3 ^ side1).norm();
    double h3 = vol / (side1 ^ side2).norm();

    return {h1, h2, h3};
}
