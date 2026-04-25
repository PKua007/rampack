//
// Created by pkua on 05.03.2022.
//

#ifndef RAMPACK_SQUAREINVERSECOREINTERACTION_H
#define RAMPACK_SQUAREINVERSECOREINTERACTION_H

#include <algorithm>
#include <cmath>

#include "CentralInteraction.h"

/**
 * @brief Parameters of square-inverse-core interaction for a pair of interaction-centre types.
 */
struct SquareInverseCorePairData {
    double epsilon{};
    double sigma{};
};

/**
 * @brief A central interaction with square-inverse-core potential.
 * @details For a pair of interaction-centre types with parameters SquareInverseCorePairData `(epsilon, sigma)`, the
 * potential is equal to `epsilon * max(0, (sigma/r)^2 - 1)`.
 */
class SquareInverseCoreInteraction : public CentralInteraction<SquareInverseCoreInteraction, SquareInverseCorePairData> {
public:
    using CentralInteraction::CentralInteraction;

    SquareInverseCoreInteraction(double epsilon, double sigma) : CentralInteraction({epsilon, sigma}) {
        Expects(epsilon != 0);
        Expects(sigma > 0);
    }

    [[nodiscard]] double calculateEnergyForDistance2(double distance2, const SquareInverseCorePairData &pairData) const
    {
        return pairData.epsilon * std::max(0.0, std::pow(pairData.sigma, 2) / distance2 - 1);
    }

    [[nodiscard]] static double getRangeRadiusForPairData(const SquareInverseCorePairData &pairData) {
        return pairData.sigma;
    }
};


#endif //RAMPACK_SQUAREINVERSECOREINTERACTION_H
