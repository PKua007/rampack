//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <ostream>
#include <numeric>

#include "Packing.h"
#include "utils/Assertions.h"

Packing::Packing(double linearSize, std::vector<std::unique_ptr<Shape>> shapes, std::unique_ptr<BoundaryConditions> bc,
                 const Interaction &interaction)
        : shapes{std::move(shapes)}, linearSize{linearSize}, bc{std::move(bc)}, interactionRange{interaction.getRangeRadius()}
{
    Expects(linearSize > 0);
    Expects(interactionRange > 0);
    Expects(!this->shapes.empty());

    this->bc->setLinearSize(this->linearSize);
    this->numInteractionCentres = interaction.getInteractionCentres().size();
    if (this->numInteractionCentres > 0) {
        this->interactionCentres.reserve(this->size() * this->numInteractionCentres);
        auto centres = interaction.getInteractionCentres();
        for (const auto &shape : this->shapes)
            for (const auto &centre : centres)
                this->interactionCentres.push_back(shape->getOrientation() * centre);
    }
    this->rebuildNeighbourGrid();
}

double Packing::tryTranslation(std::size_t particleIdx, Vector<3> translation, const Interaction &interaction) {
    Expects(particleIdx < this->size());
    Expects(interaction.getRangeRadius() <= this->interactionRange);
    this->lastAlteredParticleIdx = particleIdx;
    this->lastTranslation = translation;

    double initialEnergy = this->getParticleEnergy(particleIdx, interaction);

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            this->neighbourGrid->remove(particleIdx, this->shapes[particleIdx]->getPosition());
        } else {
            for (std::size_t i{}; i < this->numInteractionCentres; i++) {
                std::size_t idx = particleIdx * this->numInteractionCentres + i;
                auto pos = this->shapes[particleIdx]->getPosition() + this->interactionCentres[idx];
                pos += this->bc->getCorrection(pos);
                this->neighbourGrid->remove(idx, pos);
            }
        }
    }

    this->shapes[particleIdx]->translate(translation, *this->bc);

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            this->neighbourGrid->add(particleIdx, this->shapes[particleIdx]->getPosition());
        } else {
            for (std::size_t i{}; i < this->numInteractionCentres; i++) {
                std::size_t idx = particleIdx * this->numInteractionCentres + i;
                auto pos = this->shapes[particleIdx]->getPosition() + this->interactionCentres[idx];
                pos += this->bc->getCorrection(pos);
                this->neighbourGrid->add(idx, pos);
            }
        }
    }

    if (interaction.hasHardPart() && this->isAnyParticleCollidingWith(particleIdx, interaction))
        return std::numeric_limits<double>::infinity();

    double finalEnergy = this->getParticleEnergy(particleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryRotation(std::size_t particleIdx, const Matrix<3, 3> &rotation, const Interaction &interaction) {
    Expects(particleIdx < this->size());
    Expects(interaction.getRangeRadius() <= this->interactionRange);
    this->lastAlteredParticleIdx = particleIdx;
    this->lastRotation = rotation;

    double initialEnergy = this->getParticleEnergy(particleIdx, interaction);

    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0) {
        for (std::size_t i{}; i < this->numInteractionCentres; i++) {
            std::size_t idx = particleIdx * this->numInteractionCentres + i;
            auto pos = this->shapes[particleIdx]->getPosition() + this->interactionCentres[idx];
            pos += this->bc->getCorrection(pos);
            this->neighbourGrid->remove(idx, pos);
        }
    }

    this->shapes[particleIdx]->rotate(rotation);
    if (this->numInteractionCentres != 0) {
        for (std::size_t i{}; i < this->numInteractionCentres; i++) {
            std::size_t idx = particleIdx * this->numInteractionCentres + i;
            this->interactionCentres[idx] = rotation * this->interactionCentres[idx];
        }
    }

    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0) {
        for (std::size_t i{}; i < this->numInteractionCentres; i++) {
            std::size_t idx = particleIdx * this->numInteractionCentres + i;
            auto pos = this->shapes[particleIdx]->getPosition() + this->interactionCentres[idx];
            pos += this->bc->getCorrection(pos);
            this->neighbourGrid->remove(idx, pos);
        }
    }

    if (interaction.hasHardPart() && this->isAnyParticleCollidingWith(particleIdx, interaction))
        return std::numeric_limits<double>::infinity();

    double finalEnergy = this->getParticleEnergy(particleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryScaling(double scaleFactor, const Interaction &interaction) {
    Expects(scaleFactor > 0);
    Expects(interaction.getRangeRadius() <= this->interactionRange);
    this->lastScalingFactor = std::cbrt(scaleFactor);

    double initialEnergy = this->getTotalEnergy(interaction);

    this->linearSize *= this->lastScalingFactor;
    this->bc->setLinearSize(this->linearSize);
    for (auto &shape : this->shapes)
        shape->scale(this->lastScalingFactor);
    this->rebuildNeighbourGrid();
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
    if (this->neighbourGrid.has_value()) {
        for (std::size_t i{}; i < this->size(); i++)
            if (this->isAnyParticleCollidingWith(i, interaction))
                return true;
    } else {
        for (std::size_t i{}; i < this->size(); i++) {
            for (std::size_t j = i + 1; j < this->size(); j++) {
                if (this->numInteractionCentres == 0) {
                    if (interaction.overlapBetween(this->shapes[i]->getPosition(),
                                                   this->shapes[j]->getPosition(), *this->bc)) {
                        return true;
                    }
                } else {
                    for (std::size_t k{}; k < this->numInteractionCentres; k++) {
                        for (std::size_t l{}; l < this->numInteractionCentres; l++) {
                            auto pos1 = this->shapes[i]->getPosition() + this->interactionCentres[i*this->numInteractionCentres + k];
                            auto pos2 = this->shapes[j]->getPosition() + this->interactionCentres[j*this->numInteractionCentres + l];
                            if (interaction.overlapBetween(pos1, pos2, *this->bc))
                                return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool Packing::isAnyParticleCollidingWith(std::size_t i, const Interaction &interaction) const {
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            auto neigh = this->neighbourGrid->getNeighbours(this->shapes[i]->getPosition());
            for (auto j : neigh)
                if (i != j && interaction.overlapBetween(this->shapes[i]->getPosition(), this->shapes[j]->getPosition(), *this->bc))
                    return true;
        } else {
            for (std::size_t j{}; j < this->numInteractionCentres; j++) {
                std::size_t idx = i*this->numInteractionCentres + j;
                auto pos = this->shapes[i]->getPosition() + this->interactionCentres[idx];
                pos += this->bc->getCorrection(pos);
                auto neighbours = this->neighbourGrid->getNeighbours(pos);
                for (auto neigh : neighbours) {
                    std::size_t sIdx = neigh / this->numInteractionCentres;
                    if (sIdx == i)
                        continue;
                    auto pos2 = this->shapes[sIdx]->getPosition() + this->interactionCentres[neigh];
                    //pos2 += this->bc->getCorrection(pos2);
                    if (interaction.overlapBetween(pos, pos2, *this->bc))
                        return true;
                }
            }
        }
    } else {
        for (std::size_t j{}; j < this->shapes.size(); j++) {
            if (i == j)
                continue;
            if (this->numInteractionCentres == 0) {
                if (interaction.overlapBetween(this->shapes[i]->getPosition(), this->shapes[j]->getPosition(), *this->bc))
                    return true;
            } else {
                for (std::size_t k{}; k < this->numInteractionCentres; k++) {
                    for (std::size_t l{}; l < this->numInteractionCentres; l++) {
                        auto pos1 = this->shapes[i]->getPosition() + this->interactionCentres[i*this->numInteractionCentres + k];
                        auto pos2 = this->shapes[j]->getPosition() + this->interactionCentres[j*this->numInteractionCentres + l];
                        if (interaction.overlapBetween(pos1, pos2, *this->bc))
                            return true;
                    }
                }
            }
        }
    }
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
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            this->neighbourGrid->remove(lastAlteredParticleIdx, this->shapes[lastAlteredParticleIdx]->getPosition());
        } else {
            for (std::size_t i{}; i < this->numInteractionCentres; i++) {
                std::size_t idx = lastAlteredParticleIdx * this->numInteractionCentres + i;
                auto pos = this->shapes[lastAlteredParticleIdx]->getPosition() + this->interactionCentres[idx];
                pos += this->bc->getCorrection(pos);
                this->neighbourGrid->remove(idx, pos);
            }
        }
    }

    this->shapes[this->lastAlteredParticleIdx]->translate(-this->lastTranslation, *this->bc);

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            this->neighbourGrid->add(lastAlteredParticleIdx, this->shapes[lastAlteredParticleIdx]->getPosition());
        } else {
            for (std::size_t i{}; i < this->numInteractionCentres; i++) {
                std::size_t idx = lastAlteredParticleIdx * this->numInteractionCentres + i;
                auto pos = this->shapes[lastAlteredParticleIdx]->getPosition() + this->interactionCentres[idx];
                pos += this->bc->getCorrection(pos);
                this->neighbourGrid->add(idx, pos);
            }
        }
    }
}

void Packing::revertRotation() {
    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0) {
        for (std::size_t i{}; i < this->numInteractionCentres; i++) {
            std::size_t idx = lastAlteredParticleIdx * this->numInteractionCentres + i;
            auto pos = this->shapes[lastAlteredParticleIdx]->getPosition() + this->interactionCentres[idx];
            pos += this->bc->getCorrection(pos);
            this->neighbourGrid->remove(idx, pos);
        }
    }

    auto counterRot = this->lastRotation.transpose();
    this->shapes[lastAlteredParticleIdx]->rotate(counterRot);
    if (this->numInteractionCentres != 0) {
        for (std::size_t i{}; i < this->numInteractionCentres; i++) {
            std::size_t idx = lastAlteredParticleIdx * this->numInteractionCentres + i;
            this->interactionCentres[idx] = counterRot * this->interactionCentres[idx];
        }
    }

    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0) {
        for (std::size_t i{}; i < this->numInteractionCentres; i++) {
            std::size_t idx = lastAlteredParticleIdx * this->numInteractionCentres + i;
            auto pos = this->shapes[lastAlteredParticleIdx]->getPosition() + this->interactionCentres[idx];
            pos += this->bc->getCorrection(pos);
            this->neighbourGrid->add(idx, pos);
        }
    }
}

void Packing::revertScaling() {
    this->linearSize /= this->lastScalingFactor;
    this->bc->setLinearSize(this->linearSize);
    double reverseFactor = 1 / this->lastScalingFactor;
    for (auto &shape : this->shapes)
        shape->scale(reverseFactor);
    this->rebuildNeighbourGrid();
}

double Packing::getParticleEnergy([[maybe_unused]] std::size_t particleIdx, [[maybe_unused]] const Interaction &interaction) const {
    /*Expects(particleIdx < this->size());
    if (!interaction.hasSoftPart())
        return 0;

    double energy{};
    if (this->neighbourGrid.has_value()) {
        auto neigh = this->neighbourGrid->getNeighbours(this->shapes[particleIdx]->getPosition());
        for (auto i : neigh) {
            if (particleIdx == i)
                continue;
            energy += interaction.calculateEnergyBetween(*this->shapes[particleIdx], *this->shapes[i], *this->bc);
        }
    } else {
        for (std::size_t i{}; i < this->size(); i++) {
            if (particleIdx == i)
                continue;
            energy += interaction.calculateEnergyBetween(*this->shapes[particleIdx], *this->shapes[i], *this->bc);
        }
    }
    return energy;*/
    return 0;
}

double Packing::getTotalEnergy([[maybe_unused]] const Interaction &interaction) const {
    /*if (!interaction.hasSoftPart())
        return 0;

    double energy{};
    if (this->neighbourGrid.has_value()) {
        for (std::size_t i{}; i < this->size(); i++)
            energy += this->getParticleEnergy(i, interaction) / 2;  // Each interaction counted twice
    } else {
        for (std::size_t i{}; i < this->size(); i++)
            for (std::size_t j = i + 1; j < this->size(); j++)
                energy += interaction.calculateEnergyBetween(*this->shapes[i], *this->shapes[j], *this->bc);
    }
    return energy;*/
    return 0;
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
    double doubleEnergy = std::sqrt(energySum2/(N-1) - std::pow(energySum, 2)/N/(N - 1));
    return doubleEnergy / 2;    // We divide by 2, because each interaction was counted twice
}

void Packing::rebuildNeighbourGrid() {
    double cellSize = this->interactionRange;
    // linearSize/cbrt(size()) gives 1 cell per particle, factor 1/5 empirically gives best times
    double minCellSize = this->linearSize / std::cbrt(this->size()) / 5;
    if (this->interactionRange < minCellSize)
        cellSize = minCellSize;

    // Less than 4 cells in line is redundant, because everything always would be neighbour
    if (cellSize * 4 > this->linearSize) {
        this->neighbourGrid = std::nullopt;
        return;
    }

    if (!this->neighbourGrid.has_value())
        this->neighbourGrid = NeighbourGrid(this->linearSize, cellSize);
    else
        this->neighbourGrid->resize(this->linearSize, cellSize);

    for (std::size_t i{}; i < this->shapes.size(); i++) {
        if (this->numInteractionCentres == 0) {
            this->neighbourGrid->add(i, this->shapes[i]->getPosition());
        } else {
            for (std::size_t j{}; j < this->numInteractionCentres; j++) {
                std::size_t idx = i * this->numInteractionCentres + j;
                auto pos = this->shapes[i]->getPosition() + this->interactionCentres[idx];
                pos += this->bc->getCorrection(pos);
                this->neighbourGrid->add(idx, pos);
            }
        }
    }
}

void Packing::changeInteractionRange(double newRange) {
    Expects(newRange > 0);
    this->interactionRange = newRange;
    this->rebuildNeighbourGrid();
}
