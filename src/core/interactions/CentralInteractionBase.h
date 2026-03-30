//
// Created by Codex on 21/03/2026.
//

#ifndef RAMPACK_CENTRALINTERACTIONBASE_H
#define RAMPACK_CENTRALINTERACTIONBASE_H

#include "InteractionCentreLayout.h"
#include "core/Interaction.h"

/**
 * @brief A non-templated base class for central interactions exposing interaction centre layout.
 * @details The base class stores only the interaction-centre layout, while concrete pair data and the energy law are
 * provided by derived classes. The canonical spherical case is represented by the spherical
 * InteractionCentreLayout, which yields an empty Interaction::getInteractionCentres() list in accordance with the
 * Interaction contract.
 */
class CentralInteractionBase : public Interaction {
protected:
    InteractionCentreLayout interactionCentreLayout;

public:
    /**
     * @brief Returns the interaction centre layout used by the interaction.
     */
    [[nodiscard]] const InteractionCentreLayout &getInteractionCentreLayout() const {
        return this->interactionCentreLayout;
    }

    /**
     * @brief Binds the interaction to the specified interaction centre layout preserving the already configured pair
     * data.
     * @param interactionCentreLayout interaction centre positions together with centre types
     * @param allowUniformPairDataBroadcast if @a true and the interaction stores pair data only for a single centre
     * type, the derived implementation may broadcast this singular pair data to all centre types present in
     * @a interactionCentreLayout
     */
    virtual void bindCentreLayout(const InteractionCentreLayout &interactionCentreLayout,
                                  bool allowUniformPairDataBroadcast = false) = 0;

    /**
     * @brief Binds the interaction to the canonical spherical layout preserving the already configured pair data.
     * @details After calling this method, the interaction behaves as if it had a single interaction centre in the
     * origin.
     */
    void bindSphere() {
        this->bindCentreLayout(InteractionCentreLayout{});
    }

    [[nodiscard]] bool hasHardPart() const final { return false; }
    [[nodiscard]] bool hasWallPart() const final { return false; }
    [[nodiscard]] bool hasSoftPart() const final { return true; }
    [[nodiscard]] bool isConvex() const final { return false; }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const final {
        return this->interactionCentreLayout.getCentres();
    }
};


#endif //RAMPACK_CENTRALINTERACTIONBASE_H
