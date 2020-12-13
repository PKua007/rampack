//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>

#include "LatticeArrangingModel.h"
#include "utils/Assertions.h"

std::vector<std::unique_ptr<Shape>> LatticeArrangingModel::arrange(const Shape &particleMother,
                                                                   std::size_t numOfParticles, double linearSize,
                                                                   const BoundaryConditions &bc) const
{
    Expects(linearSize > 0);
    Expects((particleMother.getPosition() == std::array<double, 3>{0, 0, 0}));

    std::vector<std::unique_ptr<Shape>> result;
    result.reserve(numOfParticles);

    auto shapesInLine = static_cast<std::size_t>(std::ceil(std::cbrt(numOfParticles)));
    double factor = linearSize / shapesInLine;
    std::size_t shapeIdx{};
    for (std::size_t i{}; i < shapesInLine; i++) {
        for (std::size_t j{}; j < shapesInLine; j++) {
            for (std::size_t k{}; k < shapesInLine; k++) {
                if (shapeIdx >= numOfParticles)
                    return result;

                std::array<double, 3> translation{(i+0.5) * factor, (j+0.5) * factor, (k+0.5) * factor};
                auto particle = particleMother.clone();
                particle->translate(translation, bc);
                result.push_back(std::move(particle));
                shapeIdx++;
            }
        }
    }

    return result;
}
