//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_REPULSIVELENNARDJONESINTERACTION_H
#define RAMPACK_REPULSIVELENNARDJONESINTERACTION_H

#include "CentralInteraction.h"

struct RepulsiveLennardJonesPairData {
    double epsilon{};
    double sigma{};
};

/**
 * @brief CentralInteraction class representing Weeks-Chandler-Anderson interaction (repulsive part of LJ interaction).
 * @details It is defined as 4 * epsilon * ((r/sigma)^12 - (r/sigma)^6) + epsilon for r < 2^(1/6), 0 otherwise.
 */
class RepulsiveLennardJonesInteraction : public CentralInteraction<RepulsiveLennardJonesInteraction, RepulsiveLennardJonesPairData> {
private:
    static constexpr double WCA_CUTOFF_SIGMA_MULTIPLIER = 1.122462048309373;

public:
    using CentralInteraction<RepulsiveLennardJonesInteraction, RepulsiveLennardJonesPairData>::CentralInteraction;

    RepulsiveLennardJonesInteraction(double epsilon, double sigma)
            : CentralInteraction({epsilon, sigma})
    {
        Expects(epsilon > 0);
        Expects(sigma > 0);
    }

    [[nodiscard]] double calculateEnergyForDistance2(double distance2,
                                                     const RepulsiveLennardJonesPairData &pairData) const
    {
        double sigmaTimesTwoToOneSixth = getRangeRadiusForPairData(pairData);
        if (distance2 >= sigmaTimesTwoToOneSixth * sigmaTimesTwoToOneSixth)
            return 0;

        double x2 = pairData.sigma * pairData.sigma / distance2;
        double x6 = x2*x2*x2;
        double x12 = x6*x6;
        return 4 * pairData.epsilon * (x12 - x6) + pairData.epsilon;
    }

    [[nodiscard]] static double getRangeRadiusForPairData(const RepulsiveLennardJonesPairData &pairData)
    {
        return pairData.sigma * WCA_CUTOFF_SIGMA_MULTIPLIER;
    }
};


#endif //RAMPACK_REPULSIVELENNARDJONESINTERACTION_H
