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
 * @details The default particle selection is ParticleSelection::Mode::ALL. The selection has to be prepared for the
 * current packing size before hot-path energy evaluation uses it.
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
     */
    [[nodiscard]] virtual double calculateEnergy(const Vector<3> &shapePos,
                                                 const Matrix<3, 3> &shapeRot) const = 0;

    /**
     * @brief Returns whether the field is continuous along each box axis.
     */
    [[nodiscard]] virtual std::array<bool, 3> getContinuityAlongBoxAxes() const = 0;
};


#endif //RAMPACK_EXTERNALFIELD_H
