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
    Expects(std::all_of(dimensions.begin(), dimensions.end(), [](double d) { return d > 0; }));

    std::vector<std::unique_ptr<Shape>> result;
    result.reserve(numOfParticles);

    auto shapesInLine = static_cast<std::size_t>(std::ceil(std::cbrt(numOfParticles)));
    std::array<double, 3> factor = {dimensions[0]/shapesInLine, dimensions[1]/shapesInLine, dimensions[2]/shapesInLine};
    std::size_t shapeIdx{};
    for (std::size_t i{}; i < shapesInLine; i++) {
        for (std::size_t j{}; j < shapesInLine; j++) {
            for (std::size_t k{}; k < shapesInLine; k++) {
                if (shapeIdx >= numOfParticles)
                    return result;

                auto translation = Vector<3>{(i + 0.5) * factor[0], (j + 0.5) * factor[1], (k + 0.5) * factor[2]};
                auto particle = std::make_unique<Shape>(translation);
                result.push_back(std::move(particle));
                shapeIdx++;
            }
        }
    }

    return result;
}
