//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>

#include "PeriodicBoundaryConditions.h"
#include "utils/Assertions.h"

std::array<double, 3> PeriodicBoundaryConditions::getCorrection(const std::array<double, 3> &position) const {
    std::array<double, 3> correction{};
    correction.fill(0);

    for (std::size_t i{}; i < 3; i++) {
        while (position[i] + correction[i] < 0)
            correction[i] += this->linearSize;
        while (position[i] - correction[i] > this->linearSize)
            correction[i] -= this->linearSize;
    }
    return correction;
}

std::array<double, 3> PeriodicBoundaryConditions::getTranslation(const std::array<double, 3> &position1,
                                                                 const std::array<double, 3> &position2) const
{
    std::array<double, 3> translation{};
    translation.fill(0);

    for (std::size_t i{}; i < 3; i++) {
        while (position2[i] + translation[i] - position1[i] > this->linearSize/2)
            translation[i] -= linearSize;
        while (position2[i] + translation[i] - position1[i] < -this->linearSize/2)
            translation[i] += linearSize;
    }
    return translation;
}

double PeriodicBoundaryConditions::getDistance2(const std::array<double, 3> &position1,
                                                const std::array<double, 3> &position2) const
{
    auto translation = this->getTranslation(position1, position2);
    double distance2 = 0;
    for (std::size_t i{}; i < 3; i++)
        distance2 += std::pow(position2[i] + translation[i] - position1[i], 2);
    return distance2;
}

PeriodicBoundaryConditions::PeriodicBoundaryConditions(double linearSize) : linearSize{linearSize} {
    Expects(linearSize > 0);
}
