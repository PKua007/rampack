//
// Created by Piotr Kubala on 19/12/2020.
//

#ifndef RAMPACK_HARDCOREINTERACTION_H
#define RAMPACK_HARDCOREINTERACTION_H

#include "Interaction.h"

class HardCoreInteraction : public Interaction {
public:
    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] double calculateEnergyBetween([[maybe_unused]] const Shape &shape1,
                                                [[maybe_unused]] const Shape &shape2,
                                                [[maybe_unused]] const BoundaryConditions &bc) const override
    {
        return 0;
    }
};


#endif //RAMPACK_HARDCOREINTERACTION_H
