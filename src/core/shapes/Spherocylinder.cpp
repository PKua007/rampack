//
// Created by Piotr Kubala on 16/12/2020.
//

#include "Spherocylinder.h"
#include "core/FreeBoundaryConditions.h"


Vector<3> Spherocylinder::getEnd(short beginOrEnd, double scale) const {
    Vector<3> result;
    result[0] = 1.0;
    return scale * this->getPosition() + (this->getOrientation() * result) * (0.5 * beginOrEnd * this->length);
}

// Based on
// Copyright 2001 softSurfer, 2012 Dan Sunday
// This code may be freely used, distributed and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.
double Spherocylinder::distanceFrom(const Spherocylinder &s, double scale) const{
    Vector<3> t1 = this->getEnd(-1, scale);
    Vector<3> t2 = this->getEnd(1, scale);
    Vector<3> s1 = s.getEnd(-1, scale);
    Vector<3> s2 = s.getEnd(1, scale);

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

    return dP.norm();   // return the closest distance
}

bool Spherocylinder::overlap(const Shape &other, double scaleFactor, const BoundaryConditions &bc) const {
    Spherocylinder otherSc = dynamic_cast<const Spherocylinder &>(other);
    FreeBoundaryConditions fbc;
    otherSc.translate(bc.getTranslation(this->getPosition(), otherSc.getPosition()), fbc);
    return this->distanceFrom(otherSc, scaleFactor) < 2 * this->radius;
}

std::unique_ptr<Shape> Spherocylinder::clone() const {
    return std::make_unique<Spherocylinder>(*this);
}

double Spherocylinder::getVolume() const {
    return 2*M_PI*this->radius*this->length + 4./3*M_PI*std::pow(this->radius, 3);
}

std::string Spherocylinder::toWolfram(double scaleFactor) const {
    std::stringstream out;
    out << std::fixed;
    Vector<3> beg = this->getEnd(-1, scaleFactor);
    Vector<3> end = this->getEnd(1, scaleFactor);
    out << "CapsuleShape[{" << beg << ", " << end << "}, " << this->radius << "]";
    return out.str();
}
