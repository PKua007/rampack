//
// Created by Codex on 21/03/2026.
//

#ifndef RAMPACK_CENTRALINTERACTIONBASE_H
#define RAMPACK_CENTRALINTERACTIONBASE_H

#include "InteractionCentreLayout.h"
#include "core/Interaction.h"

/**
 * @brief A non-templated base class for central interactions exposing interaction centre layout.
 */
class CentralInteractionBase : public Interaction {
protected:
    InteractionCentreLayout interactionCentreLayout;

public:
    /**
     * @brief Returns the interaction centre layout used by the interaction.
     */
    [[nodiscard]] const InteractionCentreLayout &getInteractionCentreLayout() const
    {
        return this->interactionCentreLayout;
    }

    /**
     * @brief Binds the interaction to the specified interaction centre layout preserving the already configured pair
     * data.
     */
    virtual void bindCentreLayout(const InteractionCentreLayout &interactionCentreLayout,
                                  bool allowUniformPairDataBroadcast = false) = 0;

    /**
     * @brief Binds the interaction to the canonical spherical layout preserving the already configured pair data.
     */
    void bindSphere()
    {
        this->bindCentreLayout(InteractionCentreLayout{});
    }

    [[nodiscard]] bool hasHardPart() const final { return false; }
    [[nodiscard]] bool hasWallPart() const final { return false; }
    [[nodiscard]] bool hasSoftPart() const final { return true; }
    [[nodiscard]] bool isConvex() const final { return false; }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const final
    {
        return this->interactionCentreLayout.getCentres();
    }
};


#endif //RAMPACK_CENTRALINTERACTIONBASE_H
