//
// Created by Codex on 12/07/2026.
//

#ifndef RAMPACK_GRAVITYFIELD_H
#define RAMPACK_GRAVITYFIELD_H

#include <optional>
#include <string>

#include "core/ExternalField.h"


/**
 * @brief A box-attached linear external field.
 * @details For a particle at position @a r and orientation @a R, the energy is
 * @f[
 *     U = -g \hat{d} \cdot (r + R p - a),
 * @f]
 * where @f$p@f$ is a selected local shape point, @f$\hat{d}@f$ is the normalized absolute field direction, and @f$a@f$
 * is the absolute box anchor. The field is attached to the simulation box: its input direction is specified in
 * reciprocal/box-normal coordinates and converted to absolute coordinates by setupForBox().
 */
class GravityField : public ExternalField {
private:
    static constexpr double EPSILON = 1e-12;

    double g{};
    Vector<3> directionHkl;
    std::optional<std::string> pointName;
    Vector<3> boxAnchorRelative;

    Vector<3> localPoint;
    Vector<3> directionAbs;
    Vector<3> boxAnchorAbs;

public:
    [[nodiscard]] std::string getName() const override { return "gravity"; }

    /**
     * @brief Constructs a gravity field.
     * @param g Positive field strength.
     * @param directionHkl Non-zero field direction in reciprocal/box-normal coordinates.
     * @param pointName Optional named point used as the energy-evaluation point. If not specified,
     * setupForShapeGeometry() uses "cm" when available and otherwise falls back to the geometric origin.
     * @param boxAnchorRelative Zero-potential anchor in relative box coordinates, with all coordinates in [0, 1].
     */
    GravityField(double g, const Vector<3> &directionHkl, std::optional<std::string> pointName = std::nullopt,
                 const Vector<3> &boxAnchorRelative = {0, 0, 0});

    /**
     * @brief Resolves the local shape point used by calculateEnergy().
     * @details Explicit pointName values must exist in @a geometry. Automatic point selection prefers "cm" and then
     * falls back to ShapeGeometry::getGeometricOrigin().
     */
    void setupForShapeGeometry(const ShapeGeometry &geometry) override;

    /**
     * @brief Precomputes the absolute field direction and anchor for @a box.
     * @details The absolute direction is calculated as
     * @code
     * normalize(box.getDimensions().inverse().transpose() * directionHkl)
     * @endcode
     * and the anchor is @c box.relativeToAbsolute(boxAnchorRelative).
     */
    void setupForBox(const TriclinicBox &box) override;

    /**
     * @brief Calculates the one-particle external-field energy.
     * @details setupForShapeGeometry() and setupForBox() must be called before this method. If the selected point is
     * off-centre, @a shapeRot affects the energy.
     */
    [[nodiscard]] double calculateEnergy(const Vector<3> &shapePos, const Matrix<3, 3> &shapeRot) const override;

    /**
     * @brief Returns continuity of this field along each box axis.
     * @details The field is continuous along axis @c i only when @c directionHkl[i] is zero within EPSILON. A
     * non-continuous axis requires hard walls on that box-axis pair before the field can be used with periodic
     * simulations.
     */
    [[nodiscard]] std::array<bool, 3> getContinuityAlongBoxAxes() const override;
};


#endif //RAMPACK_GRAVITYFIELD_H
