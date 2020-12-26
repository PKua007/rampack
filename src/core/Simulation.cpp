//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <ostream>

#include "Simulation.h"
#include "utils/Assertions.h"

Simulation::Simulation(double temperature, double pressure, double positionStepSize, double rotationStepSize,
                       double volumeStepSize, std::size_t thermalisationCycles, std::size_t averagingCycles,
                       std::size_t averagingEvery, unsigned long seed)
        : temperature{temperature}, pressure{pressure}, initialTranslationStep{positionStepSize},
          initialRotationStep{rotationStepSize}, initialScalingStep{volumeStepSize},
          thermalisationCycles{thermalisationCycles}, averagingCycles{averagingCycles}, averagingEvery{averagingEvery},
          mt(seed)
{
    Expects(temperature > 0);
    Expects(pressure > 0);
    Expects(positionStepSize > 0);
    Expects(rotationStepSize > 0);
    Expects(volumeStepSize > 0);
    Expects(thermalisationCycles > 0);
    Expects(averagingCycles > 0);
    Expects(averagingEvery > 0 && averagingEvery < averagingCycles);
}

void Simulation::perform(std::unique_ptr<Packing> packing_, const Interaction &interaction, Logger &logger) {
    Expects(!packing_->empty());
    this->packing = std::move(packing_);
    this->packing->rebuildNeighbourGrid(interaction);
    this->reset();

    this->shouldAdjustStepSize = true;
    logger.setAdditionalText("thermalisation");
    logger.info() << "Starting thermalisation..." << std::endl;
    for (std::size_t i{}; i < this->thermalisationCycles * this->cycleLength; i++) {
        this->performStep(logger, interaction);
        if ((i + 1) % (this->cycleLength * this->averagingEvery) == 0) {
            this->densityThermalisationSnapshots.push_back({(i + 1) / this->cycleLength,
                                                            this->packing->getNumberDensity()});
        }
        if ((i + 1) % (this->cycleLength * 100) == 0)
            logger.info() << "Performed " << ((i + 1)/this->cycleLength) << " cycles" << std::endl;
    }

    this->shouldAdjustStepSize = false;
    logger.setAdditionalText("averaging");
    logger.info() << "Starting averaging..." << std::endl;
    for(std::size_t i{}; i < this->averagingCycles * this->cycleLength; i++) {
        this->performStep(logger, interaction);
        if ((i + 1) % (this->cycleLength * this->averagingEvery) == 0) {
            this->averagedDensities.push_back(this->packing->getNumberDensity());
            this->averagedEnergy.push_back(this->packing->getTotalEnergy(interaction));
            this->averagedEnergyFluctuations.push_back(this->packing->getParticleEnergyFluctuations(interaction));
        }
        if ((i + 1) % (this->cycleLength * 100) == 0)
            logger.info() << "Performed " << ((i + 1)/this->cycleLength) << " cycles" << std::endl;
    }

    logger.setAdditionalText("");
}

void Simulation::reset() {
    this->moveTypeDistribution = std::uniform_int_distribution<int>(0, 2 * this->packing->size());
    this->particleIdxDistribution = std::uniform_int_distribution<int>(0, this->packing->size() - 1);
    this->translationStep = this->initialTranslationStep;
    this->rotationStep = this->initialRotationStep;
    this->scalingStep = this->initialScalingStep;
    this->translationCounter.reset();
    this->rotationCounter.reset();
    this->scalingCounter.reset();
    this->averagedDensities.clear();
    this->densityThermalisationSnapshots.clear();
    this->cycleLength = 2 * this->packing->size() + 1;  // size() translations, size() rotations and 1 scaling

}

void Simulation::performStep(Logger &logger, const Interaction &interaction) {
    std::size_t moveType = this->moveTypeDistribution(this->mt);
    if (moveType == 0) {
       bool wasScaled = this->tryScaling(interaction);
       this->scalingCounter.increment(wasScaled);
    } else if (moveType <= this->packing->size()) {
        bool wasTranslated = this->tryTranslation(interaction);
        this->translationCounter.increment(wasTranslated);
    } else {
        bool wasRotated = this->tryRotation(interaction);
        this->rotationCounter.increment(wasRotated);
    }

    if (this->shouldAdjustStepSize)
        this->evaluateCounters(logger);
}

bool Simulation::tryTranslation(const Interaction &interaction) {
    Vector<3> translation{2*this->unitIntervalDistribution(this->mt) - 1,
                          2*this->unitIntervalDistribution(this->mt) - 1,
                          2*this->unitIntervalDistribution(this->mt) - 1};
    translation *= this->translationStep;

    double dE = this->packing->tryTranslation(this->particleIdxDistribution(this->mt), translation, interaction);
    if (this->unitIntervalDistribution(this->mt) <= std::exp(-dE / this->temperature)) {
        return true;
    } else {
        this->packing->revertTranslation();
        return false;
    }
}

bool Simulation::tryRotation(const Interaction &interaction) {
    Vector<3> axis;
    do {
        axis[0] = 2*this->unitIntervalDistribution(this->mt) - 1;
        axis[1] = 2*this->unitIntervalDistribution(this->mt) - 1;
        axis[2] = 2*this->unitIntervalDistribution(this->mt) - 1;
    } while (axis.norm2() > 1);
    double angle = (2*this->unitIntervalDistribution(this->mt) - 1) * this->rotationStep;
    auto rotation = Matrix<3, 3>::rotation(axis.normalized(), angle);

    double dE = this->packing->tryRotation(this->particleIdxDistribution(this->mt), rotation, interaction);
    if (this->unitIntervalDistribution(this->mt) <= std::exp(-dE / this->temperature)) {
        return true;
    } else {
        this->packing->revertRotation();
        return false;
    }
}

bool Simulation::tryScaling(const Interaction &interaction) {
    double deltaV = (2*this->unitIntervalDistribution(this->mt) - 1) * this->scalingStep;
    double currentV = std::pow(this->packing->getLinearSize(), 3);
    double factor = (deltaV + currentV) / currentV;
    if (factor < 0)
        return false;

    double N = this->packing->size();
    double dE = this->packing->tryScaling(factor, interaction);
    double exponent = N * log(factor) - dE / this->temperature - this->pressure * deltaV / this->temperature;
    if (this->unitIntervalDistribution(this->mt) <= std::exp(exponent)) {
        return true;
    } else {
        this->packing->revertScaling(interaction);
        return false;
    }
}

void Simulation::evaluateCounters(Logger &logger) {
    // Evaluate each counter every 100 cycles
    if (this->translationCounter.movesSinceEvaluation >= 100*this->packing->size()) {
        double rate = this->translationCounter.getCurrentRate();
        this->translationCounter.resetCurrent();
        if (rate > 0.5) {
            if (this->translationStep * 1.1 <= this->packing->getLinearSize()) {
                this->translationStep *= 1.1;
                logger.verbose() << "Translation rate: " << rate << ", adjusting: "  << (this->translationStep / 1.1);
                logger << " -> " << this->translationStep << std::endl;
            }
        } else if (rate < 0.3) {
            this->translationStep /= 1.1;
            logger.verbose() << "Translation rate: " << rate << ", adjusting: " << (this->translationStep * 1.1);
            logger << " -> " << (this->translationStep) << std::endl;
        }
    }

    if (this->rotationCounter.movesSinceEvaluation >= 100*this->packing->size()) {
        double rate = this->rotationCounter.getCurrentRate();
        this->rotationCounter.resetCurrent();
        if (rate > 0.5) {
            if (this->rotationStep * 1.1 <= M_PI) {
                this->rotationStep *= 1.1;
                logger.verbose() << "Rotation rate: " << rate << ", adjusting: "  << (this->rotationStep / 1.1);
                logger << " -> " << this->rotationStep << std::endl;
            }
        } else if (rate < 0.3) {
            this->rotationStep /= 1.1;
            logger.verbose() << "Rotation rate: " << rate << ", adjusting: " << (this->rotationStep * 1.1);
            logger << " -> " << (this->rotationStep) << std::endl;
        }
    }

    if (this->scalingCounter.movesSinceEvaluation >= 100) {
        double rate = this->scalingCounter.getCurrentRate();
        this->scalingCounter.resetCurrent();
        if (rate > 0.5) {
            this->scalingStep *= 1.1;
            logger.verbose() << "Scaling rate: " << rate << ", adjusting: " << (this->scalingStep / 1.1);
            logger << " -> " << this->scalingStep << std::endl;
        } else if (rate < 0.3) {
            this->scalingStep /= 1.1;
            logger.verbose() << "Scaling rate: " << rate << ", adjusting: " << (this->scalingStep * 1.1);
            logger << " -> " << this->scalingStep << std::endl;
        }
    }
}

Quantity Simulation::getAverageDensity() const {
    Quantity density;
    density.separator = Quantity::PLUS_MINUS;
    density.calculateFromSamples(this->averagedDensities);
    return density;
}

Quantity Simulation::getAverageEnergy() const {
    Quantity energy;
    energy.separator = Quantity::PLUS_MINUS;
    energy.calculateFromSamples(this->averagedEnergy);
    return energy;
}

Quantity Simulation::getAverageEnergyFluctuations() const {
    Quantity energyFluctuations;
    energyFluctuations.separator = Quantity::PLUS_MINUS;
    energyFluctuations.calculateFromSamples(this->averagedEnergyFluctuations);
    return energyFluctuations;
}

void Simulation::Counter::increment(bool accepted) {
    this->moves++;
    this->movesSinceEvaluation++;
    if (accepted) {
        this->acceptedMoves++;
        this->acceptedMovesSinceEvaluation++;
    }
}

void Simulation::Counter::reset() {
    this->acceptedMoves = 0;
    this->moves = 0;
    this->acceptedMovesSinceEvaluation = 0;
    this->movesSinceEvaluation = 0;
}

void Simulation::Counter::resetCurrent() {
    this->acceptedMovesSinceEvaluation = 0;
    this->movesSinceEvaluation = 0;
}

std::ostream &operator<<(std::ostream &out, const Simulation::ScalarSnapshot &snapshot) {
    return out << snapshot.cycleCount << " " << snapshot.value;
}
