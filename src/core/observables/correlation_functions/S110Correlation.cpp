//
// Created by pkua on 12.09.22.
//

#include "S110Correlation.h"


double S110Correlation::calculate(const Shape &shape1, const Shape &shape2, const ShapeTraits &shapeTraits) const {
    const auto &geometry = shapeTraits.getGeometry();
    switch (this->axis) {
        case ShapeGeometry::Axis::PRIMARY:
            return geometry.getPrimaryAxis(shape1) * geometry.getPrimaryAxis(shape2);
        case ShapeGeometry::Axis::SECONDARY:
            return geometry.getSecondaryAxis(shape1) * geometry.getSecondaryAxis(shape2);
        case ShapeGeometry::Axis::AUXILIARY:
            return geometry.getAuxiliaryAxis(shape1) * geometry.getAuxiliaryAxis(shape2);
        default:
            throw std::runtime_error("");
    }
}
