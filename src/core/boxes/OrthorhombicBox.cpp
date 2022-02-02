//
// Created by pkua on 02.02.2022.
//

#include <algorithm>

#include "OrthorhombicBox.h"

void OrthorhombicBox::absoluteToRelative(std::vector<Shape> &shapes) const {
    for (auto &shape : shapes)
        shape.scale(this->inverseDimensions);
}

Vector<3> OrthorhombicBox::absoluteToRelative(const Vector<3> &pos) const {
    Vector<3> result;
    for (std::size_t i{}; i < 3; i++)
        result[i] = this->inverseDimensions[i] * pos[i];
    return result;
}

void OrthorhombicBox::relativeToAbsolute(std::vector<Shape> &shapes) const {
    for (auto &shape : shapes)
        shape.scale(this->dimensions);
}

Vector<3> OrthorhombicBox::relativeToAbsolute(const Vector<3> &pos) const {
    Vector<3> result;
    for (std::size_t i{}; i < 3; i++)
        result[i] = this->dimensions[i] * pos[i];
    return result;
}

OrthorhombicBox::OrthorhombicBox(const std::array<double, 3> &dimensions) : dimensions{dimensions} {
    std::transform(this->dimensions.begin(), this->dimensions.end(), this->inverseDimensions.begin(),
                   [](double d) { return 1/d; });
}
