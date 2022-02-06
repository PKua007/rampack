//
// Created by pkua on 02.02.2022.
//

#include <algorithm>

#include "OrthorhombicBox.h"

void OrthorhombicBox::absoluteToRelative(std::vector<Shape> &shapes) const {
    for (auto &shape : shapes)
        shape.scale(this->inverseDimensions);
}

void OrthorhombicBox::relativeToAbsolute(std::vector<Shape> &shapes) const {
    for (auto &shape : shapes)
        shape.scale(this->dimensions);
}

OrthorhombicBox::OrthorhombicBox(const std::array<double, 3> &dimensions) : dimensions{dimensions} {
    std::transform(this->dimensions.begin(), this->dimensions.end(), this->inverseDimensions.begin(),
                   [](double d) { return 1/d; });
}
