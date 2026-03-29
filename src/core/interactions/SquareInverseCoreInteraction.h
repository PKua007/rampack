//
// Created by pkua on 05.03.2022.
//

#ifndef RAMPACK_SQUAREINVERSECOREINTERACTION_H
#define RAMPACK_SQUAREINVERSECOREINTERACTION_H

#include <algorithm>
#include <cmath>

#include "CentralInteraction.h"

struct SquareInverseCorePairData {
    double epsilon{};
    double sigma{};
};

class SquareInverseCoreInteraction : public CentralInteraction<SquareInverseCoreInteraction, SquareInverseCorePairData> {
public:
    using CentralInteraction<SquareInverseCoreInteraction, SquareInverseCorePairData>::CentralInteraction;

    SquareInverseCoreInteraction(double epsilon, double sigma)
            : CentralInteraction({epsilon, sigma})
    {
        Expects(epsilon != 0);
        Expects(sigma > 0);
    }

    [[nodiscard]] double calculateEnergyForDistance2(double distance2, const SquareInverseCorePairData &pairData) const
    {
        return pairData.epsilon * std::max(0.0, std::pow(pairData.sigma, 2) / distance2 - 1);
    }

    [[nodiscard]] static double getRangeRadiusForPairData(const SquareInverseCorePairData &pairData)
    {
        return pairData.sigma;
    }
};


#endif //RAMPACK_SQUAREINVERSECOREINTERACTION_H
