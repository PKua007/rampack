//
// Created by Piotr Kubala on 20/12/2020.
//

#include "SpherocylinderTraits.h"
#include "utils/Assertions.h"

SpherocylinderTraits::SpherocylinderTraits(double length, double radius) : length(length), radius(radius) {
    Expects(length >= 0);
    Expects(radius > 0);
}

Vector<3> SpherocylinderTraits::getCapCentre(short beginOrEnd, const Shape &shape, double scale) const {
    Vector<3> alignedCentre{1, 0, 0};
    return scale * shape.getPosition() + (shape.getOrientation() * alignedCentre) * (0.5 * beginOrEnd * this->length);
}

// Based on
// Copyright 2001 softSurfer, 2012 Dan Sunday
// This code may be freely used, distributed and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.
double SpherocylinderTraits::distanceBetween(const Shape &shape1, const Shape &shape2, double scale) const{
    Vector<3> t1 = this->getCapCentre(-1, shape1, scale);
    Vector<3> t2 = this->getCapCentre(1, shape1, scale);
    Vector<3> s1 = this->getCapCentre(-1, shape2, scale);
    Vector<3> s2 = this->getCapCentre(1, shape2, scale);

    Vector<3> u = t2 - t1;
    Vector<3> v = s2 - s1;
    Vector<3> w = t1 - s1;
    double a = u * u, b = u * v, c = v * v, d = u * w, e = v * w;

    double D = a * c - b * b;        // always >= 0
    double sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
    double tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

    // compute the line parameters of the two closest points
    if (D < EPSILON) { // the lines are almost parallel
        sN = 0.0;      // force using point P0 on segment S1
        sD = 1.0;      // to prevent possible division by 0.0 later
        tN = e;
        tD = c;
    } else {           // get the closest points on the infinite lines
        sN = (b * e - c * d);
        tN = (a * e - b * d);
        if (sN < 0.0) { // sc < 0 => the s=0 edge is visible
            sN = 0.0;
            tN = e;
            tD = c;
        } else if (sN > sD) {  // sc > 1  => the s=1 edge is visible
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0) { // tc < 0 => the t=0 edge is visible
        tN = 0.0;
        // recompute sc for this edge
        if (-d < 0.0)
            sN = 0.0;
        else if (-d > a)
            sN = sD;
        else {
            sN = -d;
            sD = a;
        }
    } else if (tN > tD) { // tc > 1  => the t=1 edge is visible
        tN = tD;
        // recompute sc for this edge
        if ((-d + b) < 0.0)
            sN = 0;
        else if ((-d + b) > a)
            sN = sD;
        else {
            sN = (-d + b);
            sD = a;
        }
    }
    // finally do the division to get sc and tc
    sc = (std::abs(sN) < EPSILON ? 0.0 : sN / sD);
    tc = (std::abs(tN) < EPSILON ? 0.0 : tN / tD);

    // get the difference of the two closest points
    Vector<3> dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)

    return dP.norm2();   // return the closest distance
}

bool SpherocylinderTraits::overlapBetween(const Shape &shape1, const Shape &shape2, double scaleFactor,
                                          const BoundaryConditions &bc) const
{
    Shape shape2Copy(shape2);
    shape1.applyBCTranslation(bc, shape2Copy);
    double distance2 = (shape2Copy.getPosition() - shape1.getPosition()).norm2() * scaleFactor * scaleFactor;
    if (distance2 < 4 * this->radius * this->radius)
        return true;
    else if (distance2 >= std::pow(2 * this->radius + this->length, 2))
        return false;

    return this->distanceBetween(shape1, shape2, scaleFactor) < 4 * this->radius * this->radius;
}

double SpherocylinderTraits::getVolume() const {
    return M_PI*this->radius*this->radius*this->length + 4./3*M_PI*std::pow(this->radius, 3);
}

std::string SpherocylinderTraits::toWolfram(const Shape &shape, double scaleFactor) const {
    std::stringstream out;
    out << std::fixed;
    Vector<3> beg = this->getCapCentre(-1, shape, scaleFactor);
    Vector<3> end = this->getCapCentre(1, shape, scaleFactor);
    out << "CapsuleShape[{" << beg << "," << end << "}," << this->radius << "]";
    return out.str();
}