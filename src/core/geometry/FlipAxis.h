//
// Created by Piotr Kubala on 23/02/2026.
//

#ifndef RAMPACK_FLIPAXIS_H
#define RAMPACK_FLIPAXIS_H

#include <variant>

#include "core/ShapeGeometry.h"


/**
 * @brief Axis used for flip moves, defined by a named @a ShapeGeometry axis or a dedicated orthogonal-to-primary mode.
 *
 * This class stores either one of the principal axes provided by @a ShapeGeometry or a special mode represented by
 * FlipAxis::orthogonalToPrimaryTag. The latter resolves to an axis orthogonal to the primary one using
 * @a ShapeGeometry::findFlipAxis.
 */
class FlipAxis {
public:
    /** @brief Tag struct used to select an axis orthogonal to the primary shape axis. Please use the singleton
     * FlipAxis::orthogonalToPrimaryTag instance. */
    struct OrthogonalToPrimary {
    private:
        friend class FlipAxis;
        constexpr OrthogonalToPrimary() = default;
    };

    /** @brief Tag selecting an axis orthogonal to the primary shape axis. */
    static constexpr OrthogonalToPrimary orthogonalToPrimaryTag{};

private:
    using Axis = std::variant<ShapeGeometry::Axis, OrthogonalToPrimary>;
    Axis axis{};

    [[nodiscard]] static std::string getMoveSamplerNameSuffixForShapeAxis(ShapeGeometry::Axis shapeAxis);
    [[nodiscard]] static std::string getMoveSamplerNameSuffixForOrthogonalToPrimary();

public:
    /**
     * @brief Creates the axis from named @a axis.
     * @details The constructor is implicit for easier conversion directly from ShapeGeometry::Axis.
     */
    FlipAxis(ShapeGeometry::Axis axis);

    /**
     * @brief Creates the axis from @a axis orthogonal to the primary one. Use FlipAxis::orthogonalToPrimaryTag to
     * dispatch this constructor.
     * @details The constructor is implicit for easier conversion from FlipAxis::orthogonalToPrimaryTag.
     */
    FlipAxis(OrthogonalToPrimary axis);

    /**
     * @brief Returns the move sampler name suffix for this axis.
     * @details For named shape axes, the suffix is one of: `"primary"`, `"secondary"`, or `"auxiliary"`. For the
     * orthogonal-to-primary mode, the suffix is `"orth_to_primary"`.
     */
    [[nodiscard]] std::string getMoveSamplerNameSuffix() const;

    /** @brief Returns this axis for the given @a geometry in the default shape orientation. */
    [[nodiscard]] Vector<3> getForDefaultOrientation(const ShapeGeometry &geometry) const;

    /** @brief Returns this axis for the given @a geometry and oriented @a shape. */
    [[nodiscard]] Vector<3> getForShape(const ShapeGeometry &geometry, const Shape &shape) const;
};


#endif //RAMPACK_FLIPAXIS_H
