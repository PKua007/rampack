//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <algorithm>

#include "LatticeArrangingModel.h"
#include "utils/Assertions.h"

std::vector<std::unique_ptr<Shape>> LatticeArrangingModel::arrange(std::size_t numOfParticles,
                                                                   const std::array<double, 3> &dimensions) const
{
    Expects(numOfParticles > 0);
    Expects(std::all_of(dimensions.begin(), dimensions.end(), [](double d) { return d > 0; }));

    auto particlesInLine = static_cast<std::size_t>(std::ceil(std::cbrt(numOfParticles)));
    std::array<double, 3> cellDimensions = {dimensions[0] / particlesInLine, dimensions[1] / particlesInLine, dimensions[2] / particlesInLine};
    std::array<std::size_t, 3> particlesInLineArray = {particlesInLine, particlesInLine, particlesInLine};

    return this->arrange(numOfParticles, particlesInLineArray, cellDimensions, dimensions);
}

std::vector<std::unique_ptr<Shape>>
LatticeArrangingModel::arrange(std::size_t numOfParticles, const std::array<std::size_t, 3> &particlesInLine,
                               const std::array<double, 3> &cellDimensions,
                               const std::array<double, 3> &boxDimensions) const
{
    Expects(numOfParticles > 0);
    Expects(std::all_of(particlesInLine.begin(), particlesInLine.end(), [](double d) { return d > 0; }));
    Expects(std::all_of(cellDimensions.begin(), cellDimensions.end(), [](double d) { return d > 0; }));
    Expects(std::all_of(boxDimensions.begin(), boxDimensions.end(), [](double d) { return d > 0; }));

    std::array<double, 3> offset{};
    for (std::size_t i{}; i < 3; i++) {
        offset[i] = (boxDimensions[i] - static_cast<double>(particlesInLine[i] - 1) * cellDimensions[i]) / 2;
        Expects(offset[i] > 0);
    }

    std::vector<std::unique_ptr<Shape>> result;
    result.reserve(numOfParticles);

    std::size_t shapeIdx{};
    for (std::size_t i{}; i < particlesInLine[0]; i++) {
        for (std::size_t j{}; j < particlesInLine[1]; j++) {
            for (std::size_t k{}; k < particlesInLine[2]; k++) {
                if (shapeIdx >= numOfParticles)
                    return result;

                auto translation = Vector<3>{i * cellDimensions[0] + offset[0],
                                             j * cellDimensions[1] + offset[1],
                                             k * cellDimensions[2] + offset[2]};
                auto particle = std::make_unique<Shape>(translation);
                result.push_back(std::move(particle));
                shapeIdx++;
            }
        }
    }

    return result;
}
