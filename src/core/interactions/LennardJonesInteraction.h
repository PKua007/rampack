//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_LENNARDJONESINTERACTION_H
#define RAMPACK_LENNARDJONESINTERACTION_H

#include <cmath>

#include "CentralInteraction.h"

struct LennardJonesPairData {
    double epsilon{};
    double sigma{};
};

/**
 * @brief CentralInteraction class representing Lennard-Jones interaction.
 * @details It is defined as 4 * epsilon * ((r/sigma)^12 - (r/sigma)^6)
 */
class LennardJonesInteraction : public CentralInteraction<LennardJonesInteraction, LennardJonesPairData> {
public:
    using CentralInteraction<LennardJonesInteraction, LennardJonesPairData>::CentralInteraction;

    LennardJonesInteraction(double epsilon, double sigma)
            : CentralInteraction({epsilon, sigma})
    {
        Expects(epsilon > 0);
        Expects(sigma > 0);
    }

    [[nodiscard]] double calculateEnergyForDistance2(double distance2, const LennardJonesPairData &pairData) const
    {
        double x2 = pairData.sigma * pairData.sigma / distance2;
        double x6 = x2*x2*x2;
        double x12 = x6*x6;
        return 4 * pairData.epsilon * (x12 - x6);
    }

    [[nodiscard]] static double getRangeRadiusForPairData(const LennardJonesPairData &pairData)
    {
        return 3 * pairData.sigma;
    }
};


#endif //RAMPACK_LENNARDJONESINTERACTION_H
