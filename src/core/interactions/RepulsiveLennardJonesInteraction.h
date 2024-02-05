//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_REPULSIVELENNARDJONESINTERACTION_H
#define RAMPACK_REPULSIVELENNARDJONESINTERACTION_H

#include "CentralInteraction.h"

/**
 * @brief CentralInteraction class representing Weeks-Chandler-Anderson interaction (repulsive part of LJ interaction).
 * @details It is defined as 4 * epsilon * ((r/sigma)^12 - (r/sigma)^6) + epsilon for r < 2^(1/6), 0 otherwise.
 */
class RepulsiveLennardJonesInteraction : public CentralInteraction {
private:
    double epsilon{};
    double sigma{};
    double sigmaTimesTwoToOneSixth{};

protected:
    [[nodiscard]] double calculateEnergyForDistance2(double distance2) const override;

public:
    RepulsiveLennardJonesInteraction(double epsilon, double sigma);

    [[nodiscard]] double getRangeRadius([[maybe_unused]] const std::byte *data) const override {
        return this->sigmaTimesTwoToOneSixth;
    }
};


#endif //RAMPACK_REPULSIVELENNARDJONESINTERACTION_H
