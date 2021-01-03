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
                this->interactionCentres.emplace_back(shape->getOrientation() * centre);
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
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->remove(particleIdx, this->shapes[particleIdx]->getPosition());
        else
            this->removeInteractionCentresFromNeighbourGrid(particleIdx);
    }

    this->shapes[particleIdx]->translate(translation, *this->bc);

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->add(particleIdx, this->shapes[particleIdx]->getPosition());
        else
            this->addInteractionCentresToNeighbourGrid(particleIdx);
    }

    if (interaction.hasHardPart() && this->isParticleOverlappingAnything(particleIdx, interaction))
        return std::numeric_limits<double>::infinity();

    double finalEnergy = this->getParticleEnergy(particleIdx, interaction);
    return finalEnergy - initialEnergy;
}

void Packing::addInteractionCentresToNeighbourGrid(std::size_t particleIdx) {
    for (size_t i{}; i < numInteractionCentres; i++) {
        std::size_t centreIdx = particleIdx * numInteractionCentres + i;
        auto centrePos = shapes[particleIdx]->getPosition() + interactionCentres[centreIdx];
        centrePos += bc->getCorrection(centrePos);
        neighbourGrid->add(centreIdx, centrePos);
    }
}

void Packing::removeInteractionCentresFromNeighbourGrid(std::size_t particleIdx) {
    for (size_t i{}; i < numInteractionCentres; i++) {
        std::size_t centreIdx = particleIdx * numInteractionCentres + i;
        auto centrePos = shapes[particleIdx]->getPosition() + interactionCentres[centreIdx];
        centrePos += bc->getCorrection(centrePos);
        neighbourGrid->remove(centreIdx, centrePos);
    }
}

double Packing::tryRotation(std::size_t particleIdx, const Matrix<3, 3> &rotation, const Interaction &interaction) {
    Expects(particleIdx < this->size());
    Expects(interaction.getRangeRadius() <= this->interactionRange);
    this->lastAlteredParticleIdx = particleIdx;
    this->lastRotation = rotation;

    double initialEnergy = this->getParticleEnergy(particleIdx, interaction);

    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0)
        this->removeInteractionCentresFromNeighbourGrid(particleIdx);

    this->shapes[particleIdx]->rotate(rotation);
    if (this->numInteractionCentres != 0)
        this->rotateInteractionCentres(particleIdx, rotation);

    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0)
        this->addInteractionCentresToNeighbourGrid(particleIdx);

    if (interaction.hasHardPart() && this->isParticleOverlappingAnything(particleIdx, interaction))
        return std::numeric_limits<double>::infinity();

    double finalEnergy = this->getParticleEnergy(particleIdx, interaction);
    return finalEnergy - initialEnergy;
}

void Packing::rotateInteractionCentres(std::size_t particleIdx, const Matrix<3, 3> &rotation) {
    for (size_t i{}; i < numInteractionCentres; i++) {
        size_t idx = particleIdx * numInteractionCentres + i;
        interactionCentres[idx] = rotation * interactionCentres[idx];
    }
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
            if (this->isParticleOverlappingAnything(i, interaction))
                return true;
    } else {
        for (std::size_t i{}; i < this->size(); i++)
            for (std::size_t j = i + 1; j < this->size(); j++)
                if (this->overlapBetween(i, j, interaction))
                    return true;
    }
    return false;
}

bool Packing::isParticleOverlappingAnything(std::size_t i, const Interaction &interaction) const {
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            auto neighbours = this->neighbourGrid->getNeighbours(this->shapes[i]->getPosition());
            for (auto j : neighbours) {
                if (i != j && interaction.overlapBetween(this->shapes[i]->getPosition(),
                                                         this->shapes[i]->getOrientation(),
                                                         0,
                                                         this->shapes[j]->getPosition(),
                                                         this->shapes[j]->getOrientation(),
                                                         0,
                                                         *this->bc))
                {
                    return true;
                }
            }
        } else {
            for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
                std::size_t centreIdx1 = i * this->numInteractionCentres + centre1;
                auto pos1 = this->shapes[i]->getPosition() + this->interactionCentres[centreIdx1];
                const auto &orientation1 = this->shapes[i]->getOrientation();
                pos1 += this->bc->getCorrection(pos1);
                auto neighbours = this->neighbourGrid->getNeighbours(pos1);
                for (auto centreIdx2 : neighbours) {
                    std::size_t j = centreIdx2 / this->numInteractionCentres;
                    if (j == i)
                        continue;
                    std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
                    auto pos2 = this->shapes[j]->getPosition() + this->interactionCentres[centreIdx2];
                    const auto &orientation2 = this->shapes[j]->getOrientation();
                    if (interaction.overlapBetween(pos1, orientation1, centre1, pos2, orientation2, centre2, *this->bc))
                        return true;
                }
            }
        }
    } else {
        for (std::size_t j{}; j < this->shapes.size(); j++) {
            if (i == j)
                continue;
            if (this->numInteractionCentres == 0) {
                if (interaction.overlapBetween(this->shapes[i]->getPosition(), this->shapes[i]->getOrientation(), 0,
                                               this->shapes[j]->getPosition(), this->shapes[j]->getOrientation(), 0,
                                               *this->bc))
                {
                    return true;
                }
            } else {
                for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
                    for (std::size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                        auto idx1 = i*this->numInteractionCentres + centre1;
                        auto pos1 = this->shapes[i]->getPosition() + this->interactionCentres[idx1];
                        const auto &orientation1 = this->shapes[i]->getOrientation();
                        auto idx2 = j*this->numInteractionCentres + centre2;
                        auto pos2 = this->shapes[j]->getPosition() + this->interactionCentres[idx2];
                        const auto &orientation2 = this->shapes[j]->getOrientation();
                        if (interaction.overlapBetween(pos1, orientation1, centre1,
                                                       pos2, orientation2, centre2,
                                                       *this->bc))
                        {
                            return true;
                        }
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
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->remove(lastAlteredParticleIdx, this->shapes[lastAlteredParticleIdx]->getPosition());
        else
            this->removeInteractionCentresFromNeighbourGrid(this->lastAlteredParticleIdx);
    }

    this->shapes[this->lastAlteredParticleIdx]->translate(-this->lastTranslation, *this->bc);

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->add(lastAlteredParticleIdx, this->shapes[lastAlteredParticleIdx]->getPosition());
        else
            this->addInteractionCentresToNeighbourGrid(this->lastAlteredParticleIdx);
    }
}

void Packing::revertRotation() {
    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0)
        this->removeInteractionCentresFromNeighbourGrid(this->lastAlteredParticleIdx);

    auto counterRot = this->lastRotation.transpose();
    this->shapes[this->lastAlteredParticleIdx]->rotate(counterRot);
    if (this->numInteractionCentres != 0)
        this->rotateInteractionCentres(this->lastAlteredParticleIdx, counterRot);

    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0)
        this->addInteractionCentresToNeighbourGrid(this->lastAlteredParticleIdx);
}

void Packing::revertScaling() {
    this->linearSize /= this->lastScalingFactor;
    this->bc->setLinearSize(this->linearSize);
    double reverseFactor = 1 / this->lastScalingFactor;
    for (auto &shape : this->shapes)
        shape->scale(reverseFactor);
    this->rebuildNeighbourGrid();
}

double Packing::getParticleEnergy(std::size_t particleIdx, const Interaction &interaction) const {
    Expects(particleIdx < this->size());
    if (!interaction.hasSoftPart())
        return 0;

    double energy{};
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            auto neigh = this->neighbourGrid->getNeighbours(this->shapes[particleIdx]->getPosition());
            for (auto j : neigh) {
                if (particleIdx == j)
                    continue;
                energy += interaction.calculateEnergyBetween(this->shapes[particleIdx]->getPosition(),
                                                             this->shapes[particleIdx]->getOrientation(),
                                                             0,
                                                             this->shapes[j]->getPosition(),
                                                             this->shapes[j]->getOrientation(),
                                                             0,
                                                             *this->bc);
            }
        } else {
            for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
                std::size_t centreIdx1 = particleIdx * this->numInteractionCentres + centre1;
                auto pos1 = this->shapes[particleIdx]->getPosition() + this->interactionCentres[centreIdx1];
                pos1 += this->bc->getCorrection(pos1);
                const auto &orientation1 = this->shapes[particleIdx]->getOrientation();
                auto neighbours = this->neighbourGrid->getNeighbours(pos1);
                for (auto centreIdx2 : neighbours) {
                    std::size_t j = centreIdx2 / this->numInteractionCentres;
                    if (j == particleIdx)
                        continue;
                    std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
                    auto pos2 = this->shapes[j]->getPosition() + this->interactionCentres[centreIdx2];
                    const auto &orientation2 = this->shapes[j]->getOrientation();
                    energy += interaction.calculateEnergyBetween(pos1, orientation1, centre1,
                                                                 pos2, orientation2, centre2,
                                                                 *this->bc);
                }
            }
        }
    } else {
        for (std::size_t j{}; j < this->shapes.size(); j++) {
            if (particleIdx == j)
                continue;
            if (this->numInteractionCentres == 0) {
                energy += interaction.calculateEnergyBetween(this->shapes[particleIdx]->getPosition(),
                                                             this->shapes[particleIdx]->getOrientation(),
                                                             0,
                                                             this->shapes[j]->getPosition(),
                                                             this->shapes[j]->getOrientation(),
                                                             0,
                                                             *this->bc);
            } else {
                for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
                    for (std::size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                        std::size_t centreIdx1 = particleIdx * this->numInteractionCentres + centre1;
                        auto pos1 = this->shapes[particleIdx]->getPosition() + this->interactionCentres[centreIdx1];
                        const auto &orientation1 = this->shapes[particleIdx]->getOrientation();
                        std::size_t centreIdx2 = j * this->numInteractionCentres + centre2;
                        auto pos2 = this->shapes[j]->getPosition() + this->interactionCentres[centreIdx2];
                        const auto &orientation2 = this->shapes[j]->getOrientation();
                        energy += interaction.calculateEnergyBetween(pos1, orientation1, centre1,
                                                                     pos2, orientation2, centre2,
                                                                     *this->bc);
                    }
                }
            }
        }
    }
    return energy;
}

double Packing::getTotalEnergy(const Interaction &interaction) const {
    if (!interaction.hasSoftPart())
        return 0;

    double energy{};
    if (this->neighbourGrid.has_value()) {
        for (std::size_t i{}; i < this->size(); i++)
            energy += this->getParticleEnergy(i, interaction) / 2;  // Each interaction counted twice
    } else {
        for (std::size_t i{}; i < this->size(); i++)
            for (std::size_t j = i + 1; j < this->size(); j++)
                energy += this->calculateEnergyBetween(interaction, i, j);
    }
    return energy;
}

double Packing::calculateEnergyBetween(const Interaction &interaction, std::size_t i, std::size_t j) const {
    double energy = 0;
    if (this->numInteractionCentres == 0) {
        energy += interaction.calculateEnergyBetween(this->shapes[i]->getPosition(),
                                                     this->shapes[i]->getOrientation(),
                                                     0,
                                                     this->shapes[j]->getPosition(),
                                                     this->shapes[j]->getOrientation(),
                                                     0,
                                                     *this->bc);
    } else {
        for (size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
            for (size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                size_t centreIdx1 = i * this->numInteractionCentres + centre1;
                auto pos1 = this->shapes[i]->getPosition() + this->interactionCentres[centreIdx1];
                const auto &orientation1 = this->shapes[i]->getOrientation();
                size_t centreIdx2 = j * this->numInteractionCentres + centre2;
                auto pos2 = this->shapes[j]->getPosition() + this->interactionCentres[centreIdx2];
                const auto &orientation2 =  this->shapes[j]->getOrientation();
                energy += interaction.calculateEnergyBetween(pos1, orientation1, centre1, pos2, orientation2, centre2,
                                                             *this->bc);
            }
        }
    }
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
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->add(i, this->shapes[i]->getPosition());
        else
            this->addInteractionCentresToNeighbourGrid(i);
    }
}

void Packing::changeInteractionRange(double newRange) {
    Expects(newRange > 0);
    this->interactionRange = newRange;
    this->rebuildNeighbourGrid();
}

bool Packing::overlapBetween(std::size_t particleIdx1, std::size_t particleIdx2, const Interaction &interaction) const {
    if (this->numInteractionCentres == 0) {
        if (interaction.overlapBetween(this->shapes[particleIdx1]->getPosition(),
                                       this->shapes[particleIdx1]->getOrientation(),
                                       0,
                                       this->shapes[particleIdx2]->getPosition(),
                                       this->shapes[particleIdx2]->getOrientation(),
                                       0,
                                       *this->bc))
        {
            return true;
        }
    } else {
        for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
            for (std::size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                std::size_t centreIdx1 = particleIdx1 * this->numInteractionCentres + centre1;
                std::size_t centreIdx2 = particleIdx2 * this->numInteractionCentres + centre2;
                auto pos1 = this->shapes[particleIdx1]->getPosition() + this->interactionCentres[centreIdx1];
                const auto &orientation1 = this->shapes[particleIdx1]->getOrientation();
                auto pos2 = this->shapes[particleIdx2]->getPosition() + this->interactionCentres[centreIdx2];
                const auto &orientation2 = this->shapes[particleIdx2]->getOrientation();
                if (interaction.overlapBetween(pos1, orientation1, centre1, pos2, orientation2, centre2, *this->bc))
                    return true;
            }
        }
    }
    return false;
}
