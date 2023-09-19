//
// Created by Piotr Kubala on 19/09/2023.
//

#include "ShapeAxis.h"
#include "utils/Exceptions.h"


void ShapeAxis::calculate(const Shape &shape, const ShapeTraits &traits) {
    const auto &geometry = traits.getGeometry();
    Vector<3> shapeAxis = geometry.getAxis(shape, this->axis);
    std::copy(shapeAxis.begin(), shapeAxis.end(), this->values.begin());
}

std::string ShapeAxis::getPrimaryName() const {
    switch (this->axis) {
        case ShapeGeometry::Axis::PRIMARY:
            return "pa";
        case ShapeGeometry::Axis::SECONDARY:
            return "sa";
        case ShapeGeometry::Axis::AUXILIARY:
            return "aa";
        default:
            AssertThrow("unreachable");
    }
}
