//
// Created by Piotr Kubala on 22/12/2020.
//

#include "RepulsiveLennardJonesInteraction.h"
#include "utils/Assertions.h"

double RepulsiveLennardJonesInteraction::calculateEnergyForDistance(double distance) const {
    if (distance >= this->sigmaTimesTwoToOneSixth)
        return 0;
    return 4 * this->epsilon * (std::pow(this->sigma / distance, 12) - std::pow(this->sigma / distance, 6))
           + this->epsilon;
}

RepulsiveLennardJonesInteraction::RepulsiveLennardJonesInteraction(double epsilon, double sigma)
        : epsilon{epsilon}, sigma{sigma}, sigmaTimesTwoToOneSixth{sigma * std::pow(2., 1./6)}
{
    Expects(epsilon > 0);
    Expects(sigma > 0);
}
