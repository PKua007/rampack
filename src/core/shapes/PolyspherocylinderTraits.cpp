//
// Created by Piotr Kubala on 26/04/2021.
//

#include <numeric>
#include <algorithm>
#include <iterator>

#include "PolyspherocylinderTraits.h"
#include "utils/Assertions.h"
#include "geometry/SegmentDistanceCalculator.h"

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

    std::vector<SpherocylinderData> newSpherocylinderData;
    newSpherocylinderData.reserve(this->spherocylinderData.size());
    auto massCentreShifter = [massCentre](const SpherocylinderData &data) {
        return SpherocylinderData(data.position - massCentre, data.halfAxis, data.radius);
    };
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
          circumsphereRadius{radius + halfLength}
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
    return SegmentDistanceCalculator::calculate(pos1+halfAxis1, pos1-halfAxis1, pos2bc+halfAxis2, pos2bc-halfAxis2)
           < insphereR2;
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

