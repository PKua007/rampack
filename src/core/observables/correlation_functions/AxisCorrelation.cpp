//
// Created by Piotr Kubala on 01/09/2023.
//

#include "AxisCorrelation.h"
#include "utils/Exceptions.h"


double AxisCorrelation::calculate(const Shape &shape1, const Shape &shape2, const ShapeTraits &shapeTraits) const {
    const auto &geometry = shapeTraits.getGeometry();
    switch (this->axis) {
        case ShapeGeometry::Axis::PRIMARY:
            return this->calculateForAxes(geometry.getPrimaryAxis(shape1), geometry.getPrimaryAxis(shape2));
        case ShapeGeometry::Axis::SECONDARY:
            return this->calculateForAxes(geometry.getSecondaryAxis(shape1), geometry.getSecondaryAxis(shape2));
        case ShapeGeometry::Axis::AUXILIARY:
            return this->calculateForAxes(geometry.getAuxiliaryAxis(shape1), geometry.getAuxiliaryAxis(shape2));
        default:
            AssertThrow("unreachable");
    }
}
