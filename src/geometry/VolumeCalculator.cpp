//
// Created by Piotr Kubala on 19/12/2022.
//

#include <cmath>

#include "VolumeCalculator.h"
#include "utils/Exceptions.h"


double VolumeCalculator::sphericalCap(double radius, double capHeight) {
    Expects(radius >= 0);
    Expects(capHeight <= 2*radius);

    return M_PI*capHeight*capHeight/3 * (3*radius - capHeight);
}

double VolumeCalculator::sphereIntersection(double radius1, double radius2, double distance) {
    Expects(radius1 > 0);
    Expects(radius2 > 0);
    Expects(distance > 0);
    Expects(distance <= radius1 + radius2);

    // https://en.wikipedia.org/wiki/Spherical_cap
    double r1 = radius1;
    double r2 = radius2;
    double d = distance;
    return M_PI/12/d * std::pow(r1 + r2 - d, 2) * (d*d + 2*d*(r1 + r2) - 3*std::pow(r1 - r2, 2));
}

