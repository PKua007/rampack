//
// Created by Codex on 18/07/2026.
//

#ifndef RAMPACK_PARTICLESELECTABLE_H
#define RAMPACK_PARTICLESELECTABLE_H

#include <utility>

#include "core/ParticleSelection.h"


/**
 * @brief A capability for objects whose activity can be restricted to selected particles.
 * @details The default ParticleSelection selects all particles.
 */
class ParticleSelectable {
private:
    ParticleSelection particleSelection;

public:
    virtual ~ParticleSelectable() = default;

    /**
     * @brief Sets particles eligible for this object.
     * @details The supplied selection may need to be prepared again before it is used with a packing.
     */
    void setParticleSelection(ParticleSelection selection) {
        this->particleSelection = std::move(selection);
    }

    /**
     * @brief Returns the mutable particle eligibility selection.
     */
    [[nodiscard]] ParticleSelection &getParticleSelection() {
        return this->particleSelection;
    }

    /**
     * @brief Returns the particle eligibility selection.
     */
    [[nodiscard]] const ParticleSelection &getParticleSelection() const {
        return this->particleSelection;
    }
};


#endif //RAMPACK_PARTICLESELECTABLE_H
