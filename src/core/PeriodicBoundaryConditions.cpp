//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>

#include "PeriodicBoundaryConditions.h"
#include "utils/Assertions.h"

Vector<3> PeriodicBoundaryConditions::getCorrection(const Vector<3> &position) const {
    Vector<3> correction{};

    for (std::size_t i{}; i < 3; i++) {
        while (position[i] + correction[i] < 0)
            correction[i] += this->linearSize;
        while (position[i] + correction[i] > this->linearSize)
            correction[i] -= this->linearSize;
    }
    return correction;
}

Vector<3> PeriodicBoundaryConditions::getTranslation(const Vector<3> &position1, const Vector<3> &position2) const {
    Vector<3> translation{};

    for (std::size_t i{}; i < 3; i++) {
        while (position2[i] + translation[i] - position1[i] > this->linearSize/2)
            translation[i] -= linearSize;
        while (position2[i] + translation[i] - position1[i] < -this->linearSize/2)
            translation[i] += linearSize;
    }
    return translation;
}

double PeriodicBoundaryConditions::getDistance2(const Vector<3> &position1, const Vector<3> &position2) const {
    auto translation = this->getTranslation(position1, position2);
    return (position2 + translation - position1).norm2();
}

PeriodicBoundaryConditions::PeriodicBoundaryConditions(double linearSize) : linearSize{linearSize} {
    Expects(linearSize > 0);
}
