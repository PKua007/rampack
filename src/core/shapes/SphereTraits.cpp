//
// Created by Piotr Kubala on 20/12/2020.
//

#include <cmath>
#include <sstream>

#include "SphereTraits.h"
#include "utils/Assertions.h"

SphereTraits::SphereTraits(double radius) : radius{radius} {
    Expects(radius > 0);
}

std::string SphereTraits::toWolfram(const Shape &shape, double scale) const {
    std::ostringstream out;
    out << "Sphere[" << (shape.getPosition() * scale) << "," << this->radius << "]";
    return out.str();
}

double SphereTraits::getVolume() const {
    return 4./3 * M_PI * std::pow(this->radius, 3);
}

bool SphereTraits::overlapBetween(const Shape &shape1, const Shape &shape2, double scale,
                                  const BoundaryConditions &bc) const
{
    return bc.getDistance2(shape1.getPosition(), shape2.getPosition()) < std::pow(2 * this->radius / scale, 2);
}
