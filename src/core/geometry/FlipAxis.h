//
// Created by Piotr Kubala on 23/02/2026.
//

#ifndef RAMPACK_FLIPAXIS_H
#define RAMPACK_FLIPAXIS_H

#include <variant>

#include "core/ShapeGeometry.h"


class FlipAxis {
public:
    struct OrthogonalToPrimary {
    private:
        friend class FlipAxis;
        constexpr OrthogonalToPrimary() = default;
    };

    static constexpr OrthogonalToPrimary orthogonalToPrimaryTag{};

private:
    using Axis = std::variant<ShapeGeometry::Axis, OrthogonalToPrimary>;
    Axis axis{};

    [[nodiscard]] static std::string getMoveSamplerNameSuffixForShapeAxis(ShapeGeometry::Axis shapeAxis);
    [[nodiscard]] static std::string getMoveSamplerNameSuffixForOrthogonalToPrimary();

public:
    FlipAxis(ShapeGeometry::Axis axis);
    FlipAxis(OrthogonalToPrimary axis);

    [[nodiscard]] std::string getMoveSamplerNameSuffix() const;
    [[nodiscard]] Vector<3> getForDefaultOrientation(const ShapeGeometry &geometry) const;
    [[nodiscard]] Vector<3> getForShape(const ShapeGeometry &geometry, const Shape &shape) const;
};


#endif //RAMPACK_FLIPAXIS_H
