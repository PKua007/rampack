//
// Created by Piotr Kubala on 22/02/2026.
//

#include "GeneralizedShapeAxis.h"
#include <limits>
#include <sstream>

#include "utils/Exceptions.h"


GeneralizedShapeAxis::GeneralizedShapeAxis(const Vector<3> &axis) : axis{axis} {
    constexpr double EPSILON = 1e-12;
    Expects(axis.norm2() > EPSILON * EPSILON);
    this->axis = axis.normalized();
}

GeneralizedShapeAxis::GeneralizedShapeAxis(ShapeGeometry::Axis axis) : axis{axis}
{ }

std::string GeneralizedShapeAxis::getMoveSamplerNameSuffixForShapeAxis(const ShapeGeometry::Axis shapeAxis) {
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

std::string GeneralizedShapeAxis::getMoveSamplerNameSuffixForGlobalAxis(const Vector<3> &globalAxis) {
    std::ostringstream nameOut;
    nameOut.precision(std::numeric_limits<double>::max_digits10);
    nameOut << globalAxis[0] << "," << globalAxis[1] << "," << globalAxis[2];
    return nameOut.str();
}

std::string GeneralizedShapeAxis::getMoveSamplerNameSuffix() const {
    if (const auto *shapeAxis = std::get_if<ShapeGeometry::Axis>(&this->axis))
        return getMoveSamplerNameSuffixForShapeAxis(*shapeAxis);
    if (const auto *globalAxis = std::get_if<Vector<3>>(&this->axis))
        return getMoveSamplerNameSuffixForGlobalAxis(*globalAxis);
    AssertThrow("std::variant::valueless_by_exception");
}

Vector<3> GeneralizedShapeAxis::getForDefaultOrientation(const ShapeGeometry &geometry) const {
    return this->getForShape(geometry, Shape{});
}

Vector<3> GeneralizedShapeAxis::getForShape(const ShapeGeometry &geometry, const Shape &shape) const {
    if (const auto *shapeAxis = std::get_if<ShapeGeometry::Axis>(&this->axis)) {
        return geometry.getAxis(shape, *shapeAxis);
    } else if (const auto *globalAxis = std::get_if<Vector<3>>(&this->axis)) {
        return shape.getOrientation() * *globalAxis;
    } else { // valueless_by_exception
        AssertThrow("std::variant::valueless_by_exception");
    }
}
