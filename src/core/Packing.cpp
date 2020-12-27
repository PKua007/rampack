//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <ostream>
#include <numeric>

#include "Packing.h"
#include "utils/Assertions.h"

Packing::Packing(double linearSize, std::vector<std::unique_ptr<Shape>> shapes, std::unique_ptr<BoundaryConditions> bc)
        : shapes{std::move(shapes)}, linearSize{linearSize}, bc{std::move(bc)}
{
    Expects(linearSize > 0);
    Expects(!this->shapes.empty());

    this->bc->setLinearSize(this->linearSize);
}

double Packing::tryTranslation(std::size_t particleIdx, Vector<3> translation, const Interaction &interaction) {
    Expects(particleIdx < this->size());
    this->lastAlteredParticleIdx = particleIdx;
    this->lastTranslation = translation;

    double initialEnergy = this->getParticleEnergy(particleIdx, interaction);

    if (this->neighbourGrid.has_value())
        this->neighbourGrid->remove(particleIdx, this->shapes[particleIdx]->getPosition());
    this->shapes[particleIdx]->translate(translation, *this->bc);
    if (this->neighbourGrid.has_value())
        this->neighbourGrid->add(particleIdx, this->shapes[particleIdx]->getPosition());

    if (interaction.hasHardPart() && this->isAnyParticleCollidingWith(particleIdx, interaction))
        return std::numeric_limits<double>::infinity();

    double finalEnergy = this->getParticleEnergy(particleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryRotation(std::size_t particleIdx, const Matrix<3, 3> &rotation, const Interaction &interaction) {
    Expects(particleIdx < this->size());
    this->lastAlteredParticleIdx = particleIdx;
    this->lastRotation = rotation;

    double initialEnergy = this->getParticleEnergy(particleIdx, interaction);

    this->shapes[particleIdx]->rotate(rotation);
    if (interaction.hasHardPart() && this->isAnyParticleCollidingWith(particleIdx, interaction))
        return std::numeric_limits<double>::infinity();

    double finalEnergy = this->getParticleEnergy(particleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryScaling(double scaleFactor, const Interaction &interaction) {
    Expects(scaleFactor > 0);
    this->lastScalingFactor = std::cbrt(scaleFactor);

    double initialEnergy = this->getTotalEnergy(interaction);

    this->linearSize *= this->lastScalingFactor;
    this->bc->setLinearSize(this->linearSize);
    for (auto &shape : this->shapes)
        shape->scale(this->lastScalingFactor);
    this->rebuildNeighbourGrid(interaction);
    if (interaction.hasHardPart() /*&& scaleFactor < 1*/ && this->areAnyParticlesOverlapping(interaction))
        return std::numeric_limits<double>::infinity();

    double finalEnergy = this->getTotalEnergy(interaction);
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

bool Packing::areAnyParticlesOverlapping(const Interaction &interaction) const {
    /*for (std::size_t i{}; i < this->size(); i++)
        for (std::size_t j = i + 1; j < this->size(); j++)
            if (interaction.overlapBetween(*this->shapes[i], *this->shapes[j], *this->bc))
                return true;*/
    for (std::size_t i{}; i < this->size(); i++)
        if (this->isAnyParticleCollidingWith(i, interaction))
            return true;
    return false;
}

bool Packing::isAnyParticleCollidingWith(std::size_t i, const Interaction &interaction) const {
    Assert(neighbourGrid.has_value());
    auto neigh = this->neighbourGrid->getNeighbours(this->shapes[i]->getPosition());
    for (auto j : neigh)
        if (i != j && interaction.overlapBetween(*this->shapes[i], *this->shapes[j], *this->bc))
            return true;
    return false;
}

void Packing::toWolfram(std::ostream &out, const ShapePrinter &shapePrinter) const {
    out << "Graphics3D[{" << std::endl;
    for (std::size_t i{}; i < this->shapes.size(); i++) {
        const auto &shape = shapes[i];
        out << shapePrinter.toWolfram(*shape);
        if (i != this->shapes.size() - 1)
            out << "," << std::endl;
    }
    out << "}]";
}

double Packing::getPackingFraction(double shapeVolume) const {
    Expects(shapeVolume >= 0);
    return this->getNumberDensity() * shapeVolume;
}

double Packing::getNumberDensity() const {
    return this->shapes.size() / std::pow(this->linearSize, 3);
}

void Packing::revertTranslation() {
    if (this->neighbourGrid.has_value())
        this->neighbourGrid->remove(lastAlteredParticleIdx, this->shapes[lastAlteredParticleIdx]->getPosition());
    this->shapes[this->lastAlteredParticleIdx]->translate(-this->lastTranslation, *this->bc);
    if (this->neighbourGrid.has_value())
        this->neighbourGrid->add(lastAlteredParticleIdx, this->shapes[lastAlteredParticleIdx]->getPosition());
}

void Packing::revertRotation() {
    this->shapes[this->lastAlteredParticleIdx]->rotate(this->lastRotation.transpose());
}

void Packing::revertScaling(const Interaction &interaction) {
    this->linearSize /= this->lastScalingFactor;
    this->bc->setLinearSize(this->linearSize);
    double reverseFactor = 1 / this->lastScalingFactor;
    for (auto &shape : this->shapes)
        shape->scale(reverseFactor);
    this->rebuildNeighbourGrid(interaction);
}

double Packing::getParticleEnergy(std::size_t particleIdx, const Interaction &interaction) const {
    Expects(particleIdx < this->size());
    if (!interaction.hasSoftPart())
        return 0;

    double energy{};
    for (std::size_t i{}; i < this->size(); i++) {
        if (particleIdx == i)
            continue;
        energy += interaction.calculateEnergyBetween(*this->shapes[particleIdx], *this->shapes[i], *this->bc);
    }
    return energy;
}

double Packing::getTotalEnergy(const Interaction &interaction) const {
    if (!interaction.hasSoftPart())
        return 0;

    double energy{};
    for (std::size_t i{}; i < this->size(); i++)
        for (std::size_t j = i + 1; j < this->size(); j++)
            energy += interaction.calculateEnergyBetween(*this->shapes[i], *this->shapes[j], *this->bc);
    return energy;
}

double Packing::getParticleEnergyFluctuations(const Interaction &interaction) const {
    if (!interaction.hasSoftPart())
        return 0;

    double energySum{};
    double energySum2{};
    for (std::size_t i{}; i < this->size(); i++) {
        double energy = this->getParticleEnergy(i, interaction);
        energySum += energy;
        energySum2 += energy*energy;
    }

    double N = this->size();
    return std::sqrt(energySum2/(N-1) - std::pow(energySum, 2)/N/(N - 1));
}

void Packing::rebuildNeighbourGrid(const Interaction &interaction) {
    double range = interaction.getRangeRadius();
    if (range * 3 > this->linearSize) {
        this->neighbourGrid = std::nullopt;
    } else {
        if (!this->neighbourGrid.has_value())
            this->neighbourGrid = NeighbourGrid(this->linearSize, range);
        else
            this->neighbourGrid->resize(this->linearSize, range);

        for (std::size_t i{}; i < this->shapes.size(); i++)
            this->neighbourGrid->add(i, this->shapes[i]->getPosition());
    }
}
