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

SphereTraits::SphereTraits(double radius, HardInteraction hardInteraction)
        : radius{radius}, interaction{hardInteraction}
{ }

SphereTraits::SphereTraits(double radius, LennardJonesInteraction lennardJonesInteraction)
        : radius{radius}, interaction{lennardJonesInteraction}
{ }

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
    if (!std::holds_alternative<HardInteraction>(this->interaction))
        return false;
    return bc.getDistance2(shape1.getPosition(), shape2.getPosition()) < std::pow(2 * this->radius / scale, 2);
}

double SphereTraits::calculateEnergyBetween(const Shape &shape1, const Shape &shape2, double scale,
                                            const BoundaryConditions &bc) const
{
    if (!std::holds_alternative<LennardJonesInteraction>(this->interaction))
        return false;
    double r = std::sqrt(bc.getDistance2(shape1.getPosition(), shape2.getPosition())) * scale;
    const auto &lennardJones = std::get<LennardJonesInteraction>(this->interaction);
    return 4*lennardJones.epsilon*(std::pow(lennardJones.sigma / r, 12) - std::pow(lennardJones.sigma / r, 6));
}

SphereTraits::LennardJonesInteraction::LennardJonesInteraction(double epsilon, double sigma)
        : epsilon(epsilon), sigma(sigma)
{
    Expects(epsilon > 0);
    Expects(sigma > 0);
}
