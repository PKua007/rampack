//
// Created by Piotr Kubala on 22/12/2020.
//

#include "LennardJonesInteraction.h"
#include "utils/Assertions.h"

double LennardJonesInteraction::calculateEnergyForDistance(double distance) const {
    return 4 * this->epsilon * (std::pow(this->sigma / distance, 12) - std::pow(this->sigma / distance, 6));
}

LennardJonesInteraction::LennardJonesInteraction(double epsilon, double sigma) : epsilon{epsilon}, sigma{sigma} {
    Expects(epsilon > 0);
    Expects(sigma > 0);
}
