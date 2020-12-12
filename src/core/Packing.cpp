//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>

#include "Packing.h"
#include "utils/Assertions.h"

Packing::Packing(double linearSize, std::vector<std::unique_ptr<Shape>> shapes,
                 Packing::ArrangementModel arrangementModel, std::unique_ptr<BoundaryConditions> bc)
        : shapes{std::move(shapes)}, linearSize{linearSize}, bc{std::move(bc)}
{
    Expects(linearSize > 0);
    Expects(!this->shapes.empty());

    if (arrangementModel == ArrangementModel::LATTICE_ARRANGEMENT) {
        auto shapesInLine = static_cast<std::size_t>(std::ceil(std::cbrt(this->size())));
        double factor = 1. / shapesInLine;
        std::size_t shapeIdx{};
        for (std::size_t i{}; i < shapesInLine; i++) {
            for (std::size_t j{}; j < shapesInLine; j++) {
                for (std::size_t k{}; k < shapesInLine; k++) {
                    if (shapeIdx >= this->size())
                        return;

                    std::array<double, 3> translation{(i+0.5) * factor, (j+0.5) * factor, (k+0.5) * factor};
                    this->shapes[shapeIdx]->translate(translation, *this->bc);
                    shapeIdx++;
                }
            }
        }
    }

    Ensures(!this->areAnyParticlesOverlapping());
}

bool Packing::tryTranslation(std::size_t particleIdx, std::array<double, 3> translation) {
    Expects(particleIdx < this->size());
    for (auto &coord : translation)
        coord /= this->linearSize;

    this->shapes[particleIdx]->translate(translation, *this->bc);
    if (this->isAnyParticleCollidingWith(particleIdx)) {
        for (auto &coord : translation)
            coord = -coord;
        this->shapes[particleIdx]->translate(translation, *this->bc);
        return false;
    }
    return true;
}

bool Packing::tryScaling(double scaleFactor) {
    Expects(scaleFactor > 0);
    double linearSizeSaved = this->linearSize;
    this->linearSize *= std::cbrt(scaleFactor);
    if (this->areAnyParticlesOverlapping()) {
        this->linearSize = linearSizeSaved;
        return false;
    }
    return true;
}

const Shape &Packing::operator[](std::size_t i) {
    Expects(i < this->size());
    return *this->shapes[i];
}

bool Packing::areAnyParticlesOverlapping() const {
    for (std::size_t i{}; i < this->size(); i++)
        for (std::size_t j = i + 1; j < this->size(); j++)
            if (this->shapes[i]->overlap(*this->shapes[j], this->linearSize, *this->bc))
                return true;
    return false;
}

bool Packing::isAnyParticleCollidingWith(std::size_t i) const {
    for (std::size_t j{}; j < this->size(); j++)
        if (i != j && this->shapes[i]->overlap(*this->shapes[j], this->linearSize, *this->bc))
            return true;
    return false;
}
