//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <sstream>

#include "Sphere.h"
#include "utils/Assertions.h"

bool Sphere::overlap(const Shape &other, double scaleFactor, const BoundaryConditions &bc) const {
    const auto &otherSphere = dynamic_cast<const Sphere &>(other);
    return bc.getDistance2(this->getPosition(), other.getPosition())
           < std::pow((this->radius + otherSphere.radius) / scaleFactor, 2);
}

std::unique_ptr<Shape> Sphere::clone() const {
    return std::make_unique<Sphere>(*this);
}

Sphere::Sphere(double radius) : radius{radius} {
    Expects(radius > 0);
}

std::string Sphere::toWolfram(double scaleFactor) const {
    std::ostringstream out;
    auto position = this->getPosition();
    out << "Sphere[{" << (position[0]*scaleFactor) << "," << (position[1]*scaleFactor) << ",";
    out << (position[2]*scaleFactor) << "}," << this->radius << "]";
    return out.str();
}
