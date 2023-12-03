//
// Created by Piotr Kubala on 01/09/2023.
//

#include "AxisCorrelation.h"
#include "utils/Exceptions.h"


double AxisCorrelation::calculate(const Shape &shape1, const Shape &shape2, const Vector<3> &distanceVector,
                                  const ShapeTraits &shapeTraits) const {
    const auto &geometry = shapeTraits.getGeometry();
    Vector<3> axis1, axis2;
    switch (this->axis) {
        case ShapeGeometry::Axis::PRIMARY:
            axis1 = geometry.getPrimaryAxis(shape1);
            axis2 = geometry.getPrimaryAxis(shape2);
            break;
        case ShapeGeometry::Axis::SECONDARY:
            axis1 = geometry.getSecondaryAxis(shape1);
            axis2 = geometry.getSecondaryAxis(shape2);
            break;
        case ShapeGeometry::Axis::AUXILIARY:
            axis1 = geometry.getAuxiliaryAxis(shape1);
            axis2 = geometry.getAuxiliaryAxis(shape2);
            break;
        default:
            AssertThrow("unreachable");
    }

    return this->calculateForAxes(axis1, axis2, distanceVector);
}
