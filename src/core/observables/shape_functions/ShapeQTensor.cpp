//
// Created by Piotr Kubala on 19/09/2023.
//

#include "ShapeQTensor.h"
#include "utils/Exceptions.h"


void ShapeQTensor::calculate(const Shape &shape, const ShapeTraits &traits) {
    const auto &geometry = traits.getGeometry();
    Vector<3> shapeAxis = geometry.getAxis(shape, this->axis);

    this->values = {
        1.5*shapeAxis[0]*shapeAxis[0] - 0.5,
        1.5*shapeAxis[0]*shapeAxis[1],
        1.5*shapeAxis[0]*shapeAxis[2],
        1.5*shapeAxis[1]*shapeAxis[1] - 0.5,
        1.5*shapeAxis[1]*shapeAxis[2],
        1.5*shapeAxis[2]*shapeAxis[2] - 0.5,
    };
}

std::string ShapeQTensor::getPrimaryName() const {
    switch (this->axis) {
        case ShapeGeometry::Axis::PRIMARY:
            return "Q_pa";
        case ShapeGeometry::Axis::SECONDARY:
            return "Q_sa";
        case ShapeGeometry::Axis::AUXILIARY:
            return "Q_aa";
        default:
            AssertThrow("unreachable");
    }
}
