//
// Created by Piotr Kubala on 22/02/2026.
//

#include "GeneralShapeAxis.h"
#include <limits>
#include <sstream>

#include "utils/Exceptions.h"


GeneralShapeAxis::GeneralShapeAxis(const Vector<3> &generalShapeAxis) : axis{generalShapeAxis} {
    constexpr double EPSILON = 1e-12;
    Expects(generalShapeAxis.norm2() > EPSILON * EPSILON);
    this->axis = generalShapeAxis.normalized();
}

GeneralShapeAxis::GeneralShapeAxis(ShapeGeometry::Axis shapeAxis) : axis{shapeAxis}
{ }

std::string GeneralShapeAxis::getMoveSamplerNameSuffixForShapeAxis(const ShapeGeometry::Axis shapeAxis) {
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

std::string GeneralShapeAxis::getMoveSamplerNameSuffixForGeneralShapeAxis(const Vector<3> &generalShapeAxis) {
    std::ostringstream nameOut;
    nameOut.precision(std::numeric_limits<double>::max_digits10);
    nameOut << "shape," << generalShapeAxis[0] << "," << generalShapeAxis[1] << "," << generalShapeAxis[2];
    return nameOut.str();
}

std::string GeneralShapeAxis::getMoveSamplerNameSuffix() const {
    if (const auto *shapeAxis = std::get_if<ShapeGeometry::Axis>(&this->axis))
        return getMoveSamplerNameSuffixForShapeAxis(*shapeAxis);
    if (const auto *generalShapeAxis = std::get_if<Vector<3>>(&this->axis))
        return getMoveSamplerNameSuffixForGeneralShapeAxis(*generalShapeAxis);
    AssertThrow("std::variant::valueless_by_exception");
}

Vector<3> GeneralShapeAxis::getForDefaultOrientation(const ShapeGeometry &geometry) const {
    return this->getForShape(geometry, Shape{});
}

Vector<3> GeneralShapeAxis::getForShape(const ShapeGeometry &geometry, const Shape &shape) const {
    if (const auto *shapeAxis = std::get_if<ShapeGeometry::Axis>(&this->axis))
        return geometry.getAxis(shape, *shapeAxis);
    if (const auto *generalShapeAxis = std::get_if<Vector<3>>(&this->axis))
        return shape.getOrientation() * *generalShapeAxis;
    AssertThrow("std::variant::valueless_by_exception");
}
