//
// Created by Piotr Kubala on 23/02/2026.
//

#include "FlipAxis.h"

#include "utils/Exceptions.h"


FlipAxis::FlipAxis(const ShapeGeometry::Axis axis) : axis{axis}
{ }

FlipAxis::FlipAxis(const OrthogonalToPrimary axis) : axis{axis}
{ }

std::string FlipAxis::getMoveSamplerNameSuffixForShapeAxis(const ShapeGeometry::Axis shapeAxis) {
    switch (shapeAxis) {
        case ShapeGeometry::Axis::PRIMARY:
            return "primary";
        case ShapeGeometry::Axis::SECONDARY:
            return "secondary";
        case ShapeGeometry::Axis::AUXILIARY:
            return "auxiliary";
        default:
            AssertThrow("ShapeGeometry::Axis");
    }
}

std::string FlipAxis::getMoveSamplerNameSuffixForOrthogonalToPrimary() {
    return "orth_to_primary";
}

std::string FlipAxis::getMoveSamplerNameSuffix() const {
    if (const auto *shapeAxis = std::get_if<ShapeGeometry::Axis>(&this->axis))
        return getMoveSamplerNameSuffixForShapeAxis(*shapeAxis);
    if (std::holds_alternative<OrthogonalToPrimary>(this->axis))
        return getMoveSamplerNameSuffixForOrthogonalToPrimary();
    AssertThrow("std::variant::valueless_by_exception");
}

Vector<3> FlipAxis::getForDefaultOrientation(const ShapeGeometry &geometry) const {
    return this->getForShape(geometry, Shape{});
}

Vector<3> FlipAxis::getForShape(const ShapeGeometry &geometry, const Shape &shape) const {
    if (const auto *shapeAxis = std::get_if<ShapeGeometry::Axis>(&this->axis)) {
        return geometry.getAxis(shape, *shapeAxis);
    } else if (std::holds_alternative<OrthogonalToPrimary>(this->axis)) {
        return geometry.findFlipAxis(shape);
    } else { // valueless_by_exception
        AssertThrow("std::variant::valueless_by_exception");
    }
}
