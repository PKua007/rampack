//
// Created by Piotr Kubala on 22/12/2020.
//

#include "LennardJonesInteraction.h"
#include "utils/Exceptions.h"

double LennardJonesInteraction::calculateEnergyForDistance2(double distance2) const {
    double x2 = this->sigma * this->sigma / distance2;
    double x6 = x2*x2*x2;
    double x12 = x6*x6;
    return 4 * this->epsilon * (x12 - x6);
}

LennardJonesInteraction::LennardJonesInteraction(double epsilon, double sigma) : epsilon{epsilon}, sigma{sigma} {
    Expects(epsilon > 0);
    Expects(sigma > 0);
}
