//
// Created by Piotr Kubala on 22/02/2026.
//

#ifndef RAMPACK_GENERALSHAPEAXIS_H
#define RAMPACK_GENERALSHAPEAXIS_H

#include <variant>

#include "core/ShapeGeometry.h"


class GeneralShapeAxis {
private:
    using Axis = std::variant<Vector<3>, ShapeGeometry::Axis>;
    Axis axis{};

    [[nodiscard]] static std::string getMoveSamplerNameSuffixForShapeAxis(ShapeGeometry::Axis shapeAxis);
    [[nodiscard]] static std::string getMoveSamplerNameSuffixForGeneralShapeAxis(const Vector<3> &generalShapeAxis);

public:
    explicit GeneralShapeAxis(const Vector<3> &generalShapeAxis);
    GeneralShapeAxis(ShapeGeometry::Axis shapeAxis);

    [[nodiscard]] std::string getMoveSamplerNameSuffix() const;
    [[nodiscard]] Vector<3> getForDefaultOrientation(const ShapeGeometry &geometry) const;
    [[nodiscard]] Vector<3> getForShape(const ShapeGeometry &geometry, const Shape &shape) const;
};


#endif //RAMPACK_GENERALSHAPEAXIS_H
