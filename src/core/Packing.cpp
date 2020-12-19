//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <ostream>
#include <numeric>

#include "Packing.h"
#include "utils/Assertions.h"
#include "HardShape.h"

Packing::Packing(double linearSize, std::vector<std::unique_ptr<Shape>> shapes, std::unique_ptr<BoundaryConditions> bc)
        : shapes{std::move(shapes)}, linearSize{linearSize}, bc{std::move(bc)}
{
    Expects(linearSize > 0);
    Expects(!this->shapes.empty());
    Expects(!this->areAnyParticlesOverlapping());
}

double Packing::tryTranslation(std::size_t particleIdx, Vector<3> translation, const Interaction &interaction) {
    Expects(particleIdx < this->size());
    translation /= this->linearSize;
    this->lastAlteredParticleIdx = particleIdx;
    this->lastTranslation = translation;

    double initialEnergy = (interaction.hasSoftPart() ? this->getParticleEnergy(particleIdx, interaction) : 0);

    this->shapes[particleIdx]->translate(translation, *this->bc);
    if (interaction.hasHardPart() && this->isAnyParticleCollidingWith(particleIdx))
        return std::numeric_limits<double>::infinity();

    double finalEnergy = (interaction.hasSoftPart() ? this->getParticleEnergy(particleIdx, interaction) : 0);
    return finalEnergy - initialEnergy;
}

double Packing::tryRotation(std::size_t particleIdx, const Matrix<3, 3> &rotation, const Interaction &interaction) {
    Expects(particleIdx < this->size());
    this->lastAlteredParticleIdx = particleIdx;
    this->lastRotation = rotation;

    double initialEnergy = (interaction.hasSoftPart() ? this->getParticleEnergy(particleIdx, interaction) : 0);

    this->shapes[particleIdx]->rotate(rotation);
    if (interaction.hasHardPart() && this->isAnyParticleCollidingWith(particleIdx))
        return std::numeric_limits<double>::infinity();

    double finalEnergy = (interaction.hasSoftPart() ? this->getParticleEnergy(particleIdx, interaction) : 0);
    return finalEnergy - initialEnergy;
}

double Packing::tryScaling(double scaleFactor, const Interaction &interaction) {
    Expects(scaleFactor > 0);
    this->lastScalingFactor = std::cbrt(scaleFactor);

    double initialEnergy = (interaction.hasSoftPart() ? this->getTotalEnergy(interaction) : 0);

    this->linearSize *= std::cbrt(scaleFactor);
    if (interaction.hasHardPart() && this->areAnyParticlesOverlapping())
        return std::numeric_limits<double>::infinity();

    double finalEnergy = (interaction.hasSoftPart() ? this->getTotalEnergy(interaction) : 0);
    return finalEnergy - initialEnergy;
}

const Shape &Packing::operator[](std::size_t i) const {
    Expects(i < this->size());
    return *this->shapes[i];
}

const Shape &Packing::front() const {
    Expects(!this->empty());
    return *this->shapes.front();
}

const Shape &Packing::back() const {
    Expects(!this->empty());
    return *this->shapes.back();
}

bool Packing::areAnyParticlesOverlapping() const {
    for (std::size_t i{}; i < this->size(); i++)
        for (std::size_t j = i + 1; j < this->size(); j++)
            if (this->overlapBetween(i, j))
                return true;
    return false;
}

bool Packing::isAnyParticleCollidingWith(std::size_t i) const {
    for (std::size_t j{}; j < this->size(); j++)
        if (i != j && this->overlapBetween(i, j))
            return true;
    return false;
}

void Packing::toWolfram(std::ostream &out) const {
    out << "Graphics3D[{" << std::endl;
    for (std::size_t i{}; i < this->shapes.size(); i++) {
        const auto &shape = shapes[i];
        out << shape->toWolfram(this->linearSize);
        if (i != this->shapes.size() - 1)
            out << "," << std::endl;
    }
    out << "}]";
}

double Packing::getPackingFraction() const {
    double particlesVolume = std::accumulate(this->shapes.begin(), this->shapes.end(), 0.,
                                             [](double sum, const auto &shape) { return sum + shape->getVolume(); });
    return particlesVolume / std::pow(this->linearSize, 3);
}

double Packing::getNumberDensity() const {
    return this->shapes.size() / std::pow(this->linearSize, 3);
}

bool Packing::overlapBetween(std::size_t i, std::size_t j) const {
    const auto &hardShape1 = dynamic_cast<const HardShape &>(*this->shapes[i]);
    const auto &hardShape2 = dynamic_cast<const HardShape &>(*this->shapes[j]);
    return hardShape1.overlap(hardShape2, this->linearSize, *this->bc);
}

void Packing::revertTranslation() {
    this->shapes[this->lastAlteredParticleIdx]->translate(-this->lastTranslation, *this->bc);
}

void Packing::revertRotation() {
    this->shapes[this->lastAlteredParticleIdx]->rotate(this->lastRotation.transpose());
}

void Packing::revertScaling() {
    this->linearSize /= this->lastScalingFactor;
}

double Packing::getParticleEnergy(std::size_t particleIdx, const Interaction &interaction) const {
    Expects(particleIdx < this->size());

    double energy{};
    for (std::size_t i{}; i < this->size(); i++) {
        if (particleIdx == i)
            continue;
        energy += interaction.calculateEnergyBetween(*this->shapes[particleIdx], *this->shapes[i], *this->bc);
    }
    return energy;
}

double Packing::getTotalEnergy(const Interaction &interaction) const {
    double energy{};
    for (std::size_t i{}; i < this->size(); i++)
        for (std::size_t j = i + 1; j < this->size(); j++)
            energy += interaction.calculateEnergyBetween(*this->shapes[i], *this->shapes[j], *this->bc);
    return energy;
}
