//
// Created by Piotr Kubala on 13/12/2020.
//

#include <cmath>

#include "FreeBoundaryConditions.h"

std::array<double, 3>
FreeBoundaryConditions::getCorrection([[maybe_unused]] const std::array<double, 3> &position) const
{
    std::array<double, 3> correction{};
    correction.fill(0);
    return correction;
}

std::array<double, 3>
FreeBoundaryConditions::getTranslation([[maybe_unused]] const std::array<double, 3> &position1,
                                       [[maybe_unused]] const std::array<double, 3> &position2) const
{
    std::array<double, 3> translation{};
    translation.fill(0);
    return translation;
}

double FreeBoundaryConditions::getDistance2(const std::array<double, 3> &position1,
                                            const std::array<double, 3> &position2) const {
    double distance2{};
    for (std::size_t i{}; i < 3; i++)
        distance2 += std::pow(position2[i] - position1[i], 2);
    return distance2;
}