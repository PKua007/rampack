//
// Created by pkua on 05.03.2022.
//

#include "SquareInverseCoreInteraction.h"
#include "utils/Exceptions.h"

SquareInverseCoreInteraction::SquareInverseCoreInteraction(double epsilon, double sigma) : epsilon{epsilon}, sigma{sigma} {
    Expects(epsilon != 0);
    Expects(sigma > 0);
}

double SquareInverseCoreInteraction::calculateEnergyForDistance2(double distance2) const {
    return this->epsilon * std::max(0.0, std::pow(this->sigma, 2) / distance2 - 1);
}
