//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <algorithm>

#include "OrthorombicArrangingModel.h"
#include "utils/Assertions.h"

std::vector<Shape> OrthorombicArrangingModel::arrange(std::size_t numOfParticles,
                                                      const std::array<double, 3> &dimensions) const
{
    Expects(numOfParticles > 0);
    Expects(std::all_of(dimensions.begin(), dimensions.end(), [](double d) { return d > 0; }));

    auto particlesInLine = static_cast<std::size_t>(std::ceil(std::cbrt(numOfParticles)));
    auto particlesInLineDouble = static_cast<double>(particlesInLine);
    std::array<double, 3> cellDimensions = {dimensions[0] / particlesInLineDouble,
                                            dimensions[1] / particlesInLineDouble,
                                            dimensions[2] / particlesInLineDouble};
    std::array<std::size_t, 3> particlesInLineArray = {particlesInLine, particlesInLine, particlesInLine};

    return this->arrange(numOfParticles, particlesInLineArray, cellDimensions, dimensions);
}

std::vector<Shape> OrthorombicArrangingModel::arrange(std::size_t numOfParticles,
                                                  const std::array<std::size_t, 3> &particlesInLine,
                                                  const std::array<double, 3> &cellDimensions,
                                                  const std::array<double, 3> &boxDimensions) const
{
    Expects(numOfParticles > 0);
    Expects(std::all_of(particlesInLine.begin(), particlesInLine.end(), [](double d) { return d > 0; }));
    Expects(std::all_of(cellDimensions.begin(), cellDimensions.end(), [](double d) { return d > 0; }));
    Expects(std::all_of(boxDimensions.begin(), boxDimensions.end(), [](double d) { return d > 0; }));

    if (this->polar)
        ExpectsMsg(particlesInLine[this->axisNum] % 2 == 0, "Even number of layers is needed for polar axis");

    std::array<double, 3> offset{};
    for (std::size_t i{}; i < 3; i++) {
        offset[i] = (boxDimensions[i] - static_cast<double>(particlesInLine[i] - 1) * cellDimensions[i]) / 2;
        Expects(offset[i] > 0);
    }

    std::vector<Shape> result;
    result.reserve(numOfParticles);

    std::size_t shapeIdx{};
    for (std::size_t i{}; i < particlesInLine[0]; i++) {
        for (std::size_t j{}; j < particlesInLine[1]; j++) {
            for (std::size_t k{}; k < particlesInLine[2]; k++) {
                if (shapeIdx >= numOfParticles)
                    return result;

                auto translation = Vector<3>{static_cast<double>(i) * cellDimensions[0] + offset[0],
                                             static_cast<double>(j) * cellDimensions[1] + offset[1],
                                             static_cast<double>(k) * cellDimensions[2] + offset[2]};

                std::array<std::size_t, 3> indices = {i, j, k};
                if (this->polar && indices[this->axisNum] % 2 == 1) {
                    std::array<double, 3> angles = {0, 0, 0};
                    angles[this->axisNum] = M_PI;
                    result.emplace_back(translation, Matrix<3, 3>::rotation(angles[0], angles[1], angles[2]));
                } else {
                    result.emplace_back(translation);
                }

                shapeIdx++;
            }
        }
    }

    return result;
}

OrthorombicArrangingModel::OrthorombicArrangingModel(bool polar, OrthorombicArrangingModel::PolarAxis axis)
        : polar{polar}
{
    switch (axis) {
        case PolarAxis::X: this->axisNum = 0; break;
        case PolarAxis::Y: this->axisNum = 1; break;
        case PolarAxis::Z: this->axisNum = 2; break;
    }
}
