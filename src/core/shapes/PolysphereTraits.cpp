//
// Created by Piotr Kubala on 22/12/2020.
//

#include <sstream>

#include "PolysphereTraits.h"
#include "utils/Exceptions.h"


SphereData::SphereData(const Vector<3> &position, double radius) : position{position}, radius{radius} {
    Expects(radius > 0);
}

void SphereData::toWolfram(std::ostream &out, const Shape &shape) const {
    out << "Sphere[" << this->centreForShape(shape) << "," << this->radius << "]";
}

Vector<3> SphereData::centreForShape(const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation() * this->position;
}
