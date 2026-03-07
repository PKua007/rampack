//
// Created by Piotr Kubala on 22/02/2026.
//

#ifndef RAMPACK_GENERALSHAPEAXIS_H
#define RAMPACK_GENERALSHAPEAXIS_H

#include <variant>

#include "core/ShapeGeometry.h"


/**
 * @brief Axis defined by a vector in shape coordinates or by a named @a ShapeGeometry axis.
 *
 * This class stores either a direct axis vector expressed in shape coordinates or one of the principal axes provided by
 * @a ShapeGeometry. The axis can later be resolved for the default orientation or for an oriented @a Shape.
 */
class GeneralShapeAxis {
private:
    using Axis = std::variant<Vector<3>, ShapeGeometry::Axis>;
    Axis axis{};

    [[nodiscard]] static std::string getMoveSamplerNameSuffixForShapeAxis(ShapeGeometry::Axis shapeAxis);
    [[nodiscard]] static std::string getMoveSamplerNameSuffixForGeneralShapeAxis(const Vector<3> &generalShapeAxis);

public:
    /**
     * @brief Creates the axis from @a generalShapeAxis and normalizes it if needed.
     * @details The constructor is explicit so that the caller is forced to distinguish it from a lab-frame axis
     * expressed as a plain Vector<3>.
     */
    explicit GeneralShapeAxis(const Vector<3> &generalShapeAxis);

    /**
     * @brief Creates the axis from named @a shapeAxis.
     * @details The constructor is implicit for easier conversion directly from ShapeGeometry::Axis.
     */
    GeneralShapeAxis(ShapeGeometry::Axis shapeAxis);

    /**
     * @brief Returns the move sampler name suffix for this axis.
     * @details For named shape axes, the suffix is one of: `"primary"`, `"secondary"`, or `"auxiliary"`. For direct
     * axis vectors, the suffix has the form `"shape,[x],[y],[z]"`, where `[x]`, `[y]`, and `[z]` are the normalized
     * vector components.
     */
    [[nodiscard]] std::string getMoveSamplerNameSuffix() const;

    /** @brief Returns this axis for the given @a geometry and the default shape orientation. */
    [[nodiscard]] Vector<3> getForDefaultOrientation(const ShapeGeometry &geometry) const;

    /** @brief Returns this axis for the given @a geometry and oriented @a shape. */
    [[nodiscard]] Vector<3> getForShape(const ShapeGeometry &geometry, const Shape &shape) const;
};


#endif //RAMPACK_GENERALSHAPEAXIS_H
