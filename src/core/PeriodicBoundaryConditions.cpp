//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <algorithm>

#include "PeriodicBoundaryConditions.h"
#include "utils/Assertions.h"

Vector<3> PeriodicBoundaryConditions::getCorrection(const Vector<3> &position) const {
    Vector<3> correction{};

    for (std::size_t i{}; i < 3; i++) {
        while (position[i] + correction[i] < 0)
            correction[i] += this->size[i];
        while (position[i] + correction[i] >= this->size[i])
            correction[i] -= this->size[i];
    }
    return correction;
}

Vector<3> PeriodicBoundaryConditions::getTranslation(const Vector<3> &position1, const Vector<3> &position2) const {
    Vector<3> translation{};

    for (std::size_t i{}; i < 3; i++) {
        while (position2[i] + translation[i] - position1[i] > this->size[i]/2)
            translation[i] -= this->size[i];
        while (position2[i] + translation[i] - position1[i] < -this->size[i]/2)
            translation[i] += this->size[i];
    }
    return translation;
}

double PeriodicBoundaryConditions::getDistance2(const Vector<3> &position1, const Vector<3> &position2) const {
    double distance2{};

    for (std::size_t i{}; i < 3; i++) {
        double coordDistance = std::abs(position2[i] - position1[i]);
        while (coordDistance > this->size[i] / 2)
            coordDistance -= this->size[i];
        distance2 += coordDistance * coordDistance;
    }
    return distance2;
}

PeriodicBoundaryConditions::PeriodicBoundaryConditions(double linearSize)
        : size{linearSize, linearSize, linearSize}
{
    Expects(linearSize > 0);
}

PeriodicBoundaryConditions::PeriodicBoundaryConditions(const std::array<double, 3> &size) : size{size} {
    Expects(std::all_of(size.begin(), size.end(), [](double d) { return d > 0; }));
}

void PeriodicBoundaryConditions::setLinearSize(const std::array<double, 3> &size_) {
    Expects(std::all_of(size_.begin(), size_.end(), [](double d) { return d > 0; }));
    this->size = size_;
}
