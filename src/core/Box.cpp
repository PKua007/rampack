//
// Created by pkua on 02.02.2022.
//

#include "Box.h"


std::array<Vector<3>, 3> Box::getSides() const {
    Vector<3> side1 = this->relativeToAbsolute(Vector<3>{1, 0, 0});
    Vector<3> side2 = this->relativeToAbsolute(Vector<3>{0, 1, 0});
    Vector<3> side3 = this->relativeToAbsolute(Vector<3>{0, 0, 1});

    return {side1, side2, side3};
}

double Box::getVolume() const {
    auto sides = this->getSides();
    return std::abs((sides[0] ^ sides[1]) * sides[2]);
}

std::array<double, 3> Box::getHeights() const {
    Vector<3> side1 = this->relativeToAbsolute(Vector<3>{1, 0, 0});
    Vector<3> side2 = this->relativeToAbsolute(Vector<3>{0, 1, 0});
    Vector<3> side3 = this->relativeToAbsolute(Vector<3>{0, 0, 1});
    double vol = this->getVolume();

    double h1 = vol / (side2 ^ side3).norm();
    double h2 = vol / (side3 ^ side1).norm();
    double h3 = vol / (side1 ^ side2).norm();

    return {h1, h2, h3};
}
