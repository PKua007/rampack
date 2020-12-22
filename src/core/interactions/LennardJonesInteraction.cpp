//
// Created by Piotr Kubala on 22/12/2020.
//

#include "LennardJonesInteraction.h"
#include "utils/Assertions.h"

double LennardJonesInteraction::calculateEnergyForDistance(double distance) const {
    double x = this->sigma / distance;
    double x2 = x*x;
    double x3 = x2*x;
    double x6 = x3*x3;
    double x12 = x6*x6;
    return 4 * this->epsilon * (x12 - x6);
}

LennardJonesInteraction::LennardJonesInteraction(double epsilon, double sigma) : epsilon{epsilon}, sigma{sigma} {
    Expects(epsilon > 0);
    Expects(sigma > 0);
}
