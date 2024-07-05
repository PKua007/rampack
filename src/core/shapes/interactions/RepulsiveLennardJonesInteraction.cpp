//
// Created by Piotr Kubala on 22/12/2020.
//

#include "RepulsiveLennardJonesInteraction.h"
#include "utils/Exceptions.h"

double RepulsiveLennardJonesInteraction::calculateEnergyForDistance2(double distance2) const {
    if (distance2 >= this->sigmaTimesTwoToOneSixth * this->sigmaTimesTwoToOneSixth)
        return 0;

    double x2 = this->sigma * this->sigma / distance2;
    double x6 = x2*x2*x2;
    double x12 = x6*x6;
    return 4 * this->epsilon * (x12 - x6) + this->epsilon;
}

RepulsiveLennardJonesInteraction::RepulsiveLennardJonesInteraction(double epsilon, double sigma)
        : epsilon{epsilon}, sigma{sigma}, sigmaTimesTwoToOneSixth{sigma * std::pow(2., 1./6)}
{
    Expects(epsilon > 0);
    Expects(sigma > 0);
}