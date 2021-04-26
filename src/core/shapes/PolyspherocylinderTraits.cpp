//
// Created by Piotr Kubala on 26/04/2021.
//

#include <numeric>
#include <algorithm>
#include <iterator>

#include "PolyspherocylinderTraits.h"
#include "utils/Assertions.h"

double PolyspherocylinderTraits::getVolume() const {
    return std::accumulate(this->spherocylinderData.begin(), this->spherocylinderData.end(), 0.,
                           [](double volume, const SpherocylinderData &data) { return volume + data.getVolume(); });
}

std::string PolyspherocylinderTraits::toWolfram(const Shape &shape) const {
    std::ostringstream out;
    out << std::fixed;
    out << "{";
    for (std::size_t i{}; i < this->spherocylinderData.size() - 1; i++) {
        const auto &data = this->spherocylinderData[i];
        data.toWolfram(out, shape);
        out << ",";
    }
    this->spherocylinderData.back().toWolfram(out, shape);
    out << "}";
    return out.str();
}

PolyspherocylinderTraits::PolyspherocylinderTraits(const std::vector<SpherocylinderData> &spherocylinderData,
                                                   const Vector<3> &primaryAxis, bool shouldNormalizeMassCentre)
        : spherocylinderData{spherocylinderData}, primaryAxis{primaryAxis}
{
    Expects(!spherocylinderData.empty());
    this->primaryAxis = this->primaryAxis.normalized();
    if (shouldNormalizeMassCentre)
        this->normalizeMassCentre();
}

void PolyspherocylinderTraits::normalizeMassCentre() {
    auto massCentreAccumulator = [](const Vector<3> &sum, const SpherocylinderData &data) {
        return sum + data.getVolume() * data.position;
    };
    Vector<3> massCentre = std::accumulate(this->spherocylinderData.begin(), this->spherocylinderData.end(), Vector<3>{},
                                           massCentreAccumulator);
    massCentre /= this->getVolume();

    auto massCentreShifter = [massCentre](const SpherocylinderData &data) {
        return SpherocylinderData(data.position - massCentre, data.halfAxis, data.radius);
    };

    std::vector<SpherocylinderData> newSpherocylinderData;
    newSpherocylinderData.reserve(this->spherocylinderData.size());
    std::transform(this->spherocylinderData.begin(), this->spherocylinderData.end(),
                   std::back_inserter(newSpherocylinderData), massCentreShifter);
    this->spherocylinderData = std::move(newSpherocylinderData);
}

Vector<3> PolyspherocylinderTraits::getPrimaryAxis(const Shape &shape) const {
    return shape.getOrientation() * this->primaryAxis;
}

PolyspherocylinderTraits::SpherocylinderData::SpherocylinderData(const Vector<3> &position, const Vector<3> &halfAxis,
                                                                 double radius)
        : position{position}, halfAxis{halfAxis}, halfLength{halfAxis.norm()}, radius{radius},
          circumsphereRadius{radius + halfLength},
          maxDistance{std::max((position + halfAxis).norm(), (position - halfAxis).norm()) + radius}
{
    Expects(radius > 0);
}

void PolyspherocylinderTraits::SpherocylinderData::toWolfram(std::ostream &out, const Shape &shape) const {
    Vector<3> shapeHalfAxis = this->halfAxisForShape(shape);
    Vector<3> beg = shape.getPosition() + shape.getOrientation() * this->position + shapeHalfAxis;
    Vector<3> end = shape.getPosition() + shape.getOrientation() * this->position - shapeHalfAxis;

    out << "Tube[{" << beg << "," << end << "}," << this->radius << "]";
}

Vector<3> PolyspherocylinderTraits::SpherocylinderData::centreForShape(const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation() * this->position;
}

double PolyspherocylinderTraits::SpherocylinderData::getVolume() const {
    double radius2 = this->radius * this->radius;
    double radius3 = radius2 * this->radius;
    double axisLength = 2 * this->halfLength;
    return 4*M_PI/3*radius3 + M_PI*radius2*axisLength;
}

Vector<3> PolyspherocylinderTraits::SpherocylinderData::halfAxisForShape(const Shape &shape) const {
    return shape.getOrientation() * this->halfAxis;
}

bool PolyspherocylinderTraits::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                              const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                              const BoundaryConditions &bc) const
{
    const auto &data1 = this->spherocylinderData[idx1];
    const auto &data2 = this->spherocylinderData[idx2];

    Vector<3> pos2bc = pos2 + bc.getTranslation(pos1, pos2);
    double distance2 = (pos2bc - pos1).norm2();
    double insphereR = data1.radius + data2.radius;
    double insphereR2 = insphereR * insphereR;
    if (distance2 < insphereR2)
        return true;
    double circumsphereR = data1.circumsphereRadius + data2.circumsphereRadius;
    double circumsphereR2 = circumsphereR * circumsphereR;
    if (distance2 >= circumsphereR2)
        return false;

    Vector<3> halfAxis1 = orientation1 * data1.halfAxis;
    Vector<3> halfAxis2 = orientation2 * data2.halfAxis;
    return distance2Between(pos1 + halfAxis1, pos1 - halfAxis1, pos2bc + halfAxis2, pos2bc - halfAxis2) < insphereR2;
}

std::vector<Vector<3>> PolyspherocylinderTraits::getInteractionCentres() const {
    std::vector<Vector<3>> centres;
    centres.reserve(this->spherocylinderData.size());
    for (const auto &data : this->spherocylinderData)
        centres.push_back(data.position);
    return centres;
}

double PolyspherocylinderTraits::getRangeRadius() const {
    auto comparator = [](const SpherocylinderData &sd1, const SpherocylinderData &sd2) {
        return sd1.circumsphereRadius < sd2.circumsphereRadius;
    };
    auto maxIt = std::max_element(this->spherocylinderData.begin(), this->spherocylinderData.end(), comparator);
    return 2 * maxIt->circumsphereRadius;
}

// Based on
// Copyright 2001 softSurfer, 2012 Dan Sunday
// This code may be freely used, distributed and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.
double PolyspherocylinderTraits::distance2Between(const Vector<3> &capCentre11, const Vector<3> &capCentre12,
                                                  const Vector<3> &capCentre21, const Vector<3> &capCentre22)
{
    Vector<3> u = capCentre12 - capCentre11;
    Vector<3> v = capCentre22 - capCentre21;
    Vector<3> w = capCentre11 - capCentre21;
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
