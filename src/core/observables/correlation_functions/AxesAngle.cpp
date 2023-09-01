//
// Created by Piotr Kubala on 20/04/2023.
//

#include "AxesAngle.h"


double AxesAngle::calculate(const Shape &shape1, const Shape &shape2, [[maybe_unused]] const Vector<3> &distanceVector,
                            const ShapeTraits &shapeTraits) const {
    const auto &geometry = shapeTraits.getGeometry();
    double cosine{};
    switch (this->axis) {
        case ShapeGeometry::Axis::PRIMARY:
            cosine = geometry.getPrimaryAxis(shape1) * geometry.getPrimaryAxis(shape2);
            break;
        case ShapeGeometry::Axis::SECONDARY:
            cosine = geometry.getSecondaryAxis(shape1) * geometry.getSecondaryAxis(shape2);
            break;
        case ShapeGeometry::Axis::AUXILIARY:
            cosine = geometry.getAuxiliaryAxis(shape1) * geometry.getAuxiliaryAxis(shape2);
            break;
        default:
            throw std::runtime_error("");
    }

    return 180 * std::acos(std::abs(cosine)) / M_PI;
}
