//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <algorithm>

#include "PeriodicBoundaryConditions.h"
#include "utils/Assertions.h"

Vector<3> PeriodicBoundaryConditions::getCorrection(const Vector<3> &position) const {
    Vector<3> positionRel = this->box.absoluteToRelative(position);
    Vector<3> correctionRel{};

    for (std::size_t i{}; i < 3; i++) {
        while (positionRel[i] + correctionRel[i] < 0)
            correctionRel[i] += 1;
        while (positionRel[i] + correctionRel[i] >= 1)
            correctionRel[i] -= 1;
    }
    return this->box.relativeToAbsolute(correctionRel);
}

Vector<3> PeriodicBoundaryConditions::getTranslation(const Vector<3> &position1, const Vector<3> &position2) const {
    Vector<3> positionRel1 = this->box.absoluteToRelative(position1);
    Vector<3> positionRel2 = this->box.absoluteToRelative(position2);
    Vector<3> translationRel{};

    for (std::size_t i{}; i < 3; i++) {
        while (positionRel2[i] + translationRel[i] - positionRel1[i] > 0.5)
            translationRel[i] -= 1;
        while (positionRel2[i] + translationRel[i] - positionRel1[i] < -0.5)
            translationRel[i] += 1;
    }
    return this->box.relativeToAbsolute(translationRel);
}

double PeriodicBoundaryConditions::getDistance2(const Vector<3> &position1, const Vector<3> &position2) const {
    Vector<3> positionRel1 = this->box.absoluteToRelative(position1);
    Vector<3> positionRel2 = this->box.absoluteToRelative(position2);

    Vector<3> distanceRel;
    for (std::size_t i{}; i < 3; i++) {
        double coordDistanceRel = std::abs(positionRel2[i] - positionRel1[i]);
        while (coordDistanceRel > 0.5)
            coordDistanceRel -= 1;
        distanceRel[i] = coordDistanceRel;
    }
    return this->box.relativeToAbsolute(distanceRel).norm2();
}

PeriodicBoundaryConditions::PeriodicBoundaryConditions(const TriclinicBox &box) : box{box} {
    Expects(box.getVolume() != 0);
}

void PeriodicBoundaryConditions::setBox(const TriclinicBox &box_) {
    Expects(box_.getVolume() != 0);
    this->box = box_;
}
