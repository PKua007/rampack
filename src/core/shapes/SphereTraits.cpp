//
// Created by Piotr Kubala on 20/12/2020.
//

#include <cmath>
#include <sstream>

#include "SphereTraits.h"
#include "utils/Assertions.h"

SphereTraits::SphereTraits(double radius) : radius{radius}, interaction{std::make_unique<HardInteraction>(radius)} {
    Expects(radius > 0);
}

SphereTraits::SphereTraits(double radius, std::unique_ptr<CentralInteraction> centralInteraction)
        : radius{radius}
{
    Expects(radius > 0);
    centralInteraction->installOnSphere();
    this->interaction = std::move(centralInteraction);
}

std::string SphereTraits::toWolfram(const Shape &shape) const {
    std::ostringstream out;
    out << "Sphere[" << (shape.getPosition()) << "," << this->radius << "]";
    return out.str();
}

double SphereTraits::getVolume() const {
    return 4./3 * M_PI * std::pow(this->radius, 3);
}

bool SphereTraits::HardInteraction::overlapBetween(const Vector<3> &shape1, const Vector<3> &shape2,
                                                   const BoundaryConditions &bc) const
{
    return bc.getDistance2(shape1, shape2) < std::pow(2 * this->radius, 2);
}