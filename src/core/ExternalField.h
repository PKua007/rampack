//
// Created by Codex on 12/07/2026.
//

#ifndef RAMPACK_EXTERNALFIELD_H
#define RAMPACK_EXTERNALFIELD_H

#include <array>
#include <string>

#include "geometry/Matrix.h"
#include "geometry/Vector.h"
#include "core/ParticleSelectable.h"
#include "core/ShapeGeometry.h"
#include "core/TriclinicBox.h"


/**
 * @brief An interface representing a one-body external field contribution to particle energy.
 * @details The default particle selection is ParticleSelection::Mode::ALL.
 */
class ExternalField : public ParticleSelectable {
public:
    /**
     * @brief Returns the external-field name used in diagnostics.
     */
    [[nodiscard]] virtual std::string getName() const = 0;

    /**
     * @brief Prepares shape-geometry dependent data used by the field.
     */
    virtual void setupForShapeGeometry(const ShapeGeometry &geometry) = 0;

    /**
     * @brief Prepares box-dependent data used by the field.
     */
    virtual void setupForBox(const TriclinicBox &box) = 0;

    /**
     * @brief Calculates external-field energy for a shape position and orientation.
     * @details Implementations must be safe for concurrent const calls and should not allocate. Both setup methods must
     * have been called for the current shape geometry and box.
     */
    [[nodiscard]] virtual double calculateEnergy(const Vector<3> &shapePos,
                                                 const Matrix<3, 3> &shapeRot) const = 0;

    /**
     * @brief Returns whether the field is continuous along each box axis.
     * @details A false value for an axis means that periodic wrapping across its wall pair changes the potential and
     * therefore hard walls are required on that axis.
     */
    [[nodiscard]] virtual std::array<bool, 3> getContinuityAlongBoxAxes() const = 0;
};


#endif //RAMPACK_EXTERNALFIELD_H
