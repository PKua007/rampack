//
// Created by Piotr Kubala on 22/02/2026.
//

#ifndef RAMPACK_GENERALIZEDSHAPEAXIS_H
#define RAMPACK_GENERALIZEDSHAPEAXIS_H

#include <variant>

#include "core/ShapeGeometry.h"


class GeneralizedShapeAxis {
private:
    using Axis = std::variant<Vector<3>, ShapeGeometry::Axis>;
    Axis axis{};

    [[nodiscard]] static std::string getMoveSamplerNameSuffixForShapeAxis(ShapeGeometry::Axis shapeAxis);
    [[nodiscard]] static std::string getMoveSamplerNameSuffixForGlobalAxis(const Vector<3> &globalAxis);

public:
    GeneralizedShapeAxis(const Vector<3> &axis);
    GeneralizedShapeAxis(ShapeGeometry::Axis axis);

    [[nodiscard]] std::string getMoveSamplerNameSuffix() const;
    [[nodiscard]] Vector<3> getForDefaultOrientation(const ShapeGeometry &geometry) const;
    [[nodiscard]] Vector<3> getForShape(const ShapeGeometry &geometry, const Shape &shape) const;
};


#endif //RAMPACK_GENERALIZEDSHAPEAXIS_H
