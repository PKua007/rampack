//
// Created by Piotr Kubala on 22/12/2020.
//

#include "RepulsiveLennardJonesInteraction.h"
#include "utils/Assertions.h"

double RepulsiveLennardJonesInteraction::calculateEnergyForDistance(double distance) const {
    if (distance >= this->sigmaTimesTwoToOneSixth)
        return 0;

    double x = this->sigma / distance;
    double x2 = x*x;
    double x3 = x2*x;
    double x6 = x3*x3;
    double x12 = x6*x6;
    return 4 * this->epsilon * (x12 - x6) + this->epsilon;
}

RepulsiveLennardJonesInteraction::RepulsiveLennardJonesInteraction(double epsilon, double sigma)
        : epsilon{epsilon}, sigma{sigma}, sigmaTimesTwoToOneSixth{sigma * std::pow(2., 1./6)}
{
    Expects(epsilon > 0);
    Expects(sigma > 0);
}
