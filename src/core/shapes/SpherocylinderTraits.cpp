//
// Created by Piotr Kubala on 20/12/2020.
//

#include "SpherocylinderTraits.h"
#include "utils/Assertions.h"
#include "geometry/SegmentDistanceCalculator.h"

SpherocylinderTraits::SpherocylinderTraits(double length, double radius) : length{length}, radius{radius} {
    Expects(length >= 0);
    Expects(radius > 0);

    this->registerNamedPoint("beg", this->getCapCentre(-1, {}));
    this->registerNamedPoint("end", this->getCapCentre(1, {}));
}

Vector<3> SpherocylinderTraits::getCapCentre(short beginOrEnd, const Shape &shape) const {
    Vector<3> alignedCentre{1, 0, 0};
    return shape.getPosition() + (shape.getOrientation() * alignedCentre) * (0.5 * beginOrEnd * this->length);
}

bool SpherocylinderTraits::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                          [[maybe_unused]] std::size_t idx1, const Vector<3> &pos2,
                                          const Matrix<3, 3> &orientation2, [[maybe_unused]] std::size_t idx2,
                                          const BoundaryConditions &bc) const
{
    Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
    double distance2 = (pos2bc - pos1).norm2();
    if (distance2 < 4 * this->radius * this->radius)
        return true;
    else if (distance2 >= std::pow(2 * this->radius + this->length, 2))
        return false;

    Shape shape1(pos1, orientation1);
    Shape shape2(pos2bc, orientation2);

    return SegmentDistanceCalculator::calculate(this->getCapCentre(-1, shape1), this->getCapCentre(1, shape1),
                                                this->getCapCentre(-1, shape2),this->getCapCentre(1, shape2))
           < 4 * this->radius * this->radius;
}

double SpherocylinderTraits::getVolume() const {
    return M_PI*this->radius*this->radius*this->length + 4./3*M_PI*std::pow(this->radius, 3);
}

std::string SpherocylinderTraits::toWolfram(const Shape &shape) const {
    std::stringstream out;
    out << std::fixed;
    Vector<3> beg = this->getCapCentre(-1, shape);
    Vector<3> end = this->getCapCentre(1, shape);
    out << "CapsuleShape[{" << beg << "," << end << "}," << this->radius << "]";
    return out.str();
}

Vector<3> SpherocylinderTraits::getPrimaryAxis(const Shape &shape) const {
    return shape.getOrientation().column(0);
}

bool SpherocylinderTraits::overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation,
                                           [[maybe_unused]] std::size_t idx, const Vector<3> &wallOrigin,
                                           const Vector<3> &wallVector) const
{
    Vector<3> halfAxis = orientation.column(0) * (this->length/2);

    Vector<3> cap1 = pos + halfAxis;
    double dotProduct1 = wallVector * (cap1 - wallOrigin);
    if (dotProduct1 < this->radius)
        return true;

    Vector<3> cap2 = pos - halfAxis;
    double dotProduct2 = wallVector * (cap2 - wallOrigin);
    if (dotProduct2 < this->radius)
        return true;

    return false;
}

const ShapePrinter &SpherocylinderTraits::getPrinter(const std::string &format) const {
    ExpectsMsg(format == "wolfram", "SpherocylinderTraits: unknown printer format: " + format);
    return *this;
}
