//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>

#include "Sphere.h"
#include "utils/Assertions.h"

bool Sphere::overlap(const Shape &other, double scaleFactor, const BoundaryConditions &bc) const {
    return bc.getDistance2(this->getPosition(), other.getPosition()) < std::pow(2 * this->radius / scaleFactor, 2);
}

std::unique_ptr<Shape> Sphere::clone() const {
    return std::make_unique<Sphere>(*this);
}

Sphere::Sphere(double radius) : radius{radius} {
    Expects(radius > 0);
}
