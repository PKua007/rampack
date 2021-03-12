//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <ostream>
#include <chrono>

#include "Simulation.h"
#include "utils/Assertions.h"

Simulation::Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep,
                       double scalingStep, unsigned long seed)
        : translationStep{translationStep}, rotationStep{rotationStep}, scalingStep{scalingStep}, mt(seed),
          packing{std::move(packing)}
{
    Expects(!this->packing->empty());
    Expects(translationStep > 0);
    Expects(rotationStep > 0);
    Expects(scalingStep > 0);
}

void Simulation::perform(double temperature_, double pressure_, std::size_t thermalisationCycles_,
                         std::size_t averagingCycles_, std::size_t averagingEvery_, const Interaction &interaction,
                         Logger &logger)
{
    Expects(temperature_ > 0);
    Expects(pressure_ > 0);
    Expects(thermalisationCycles_ > 0);
    Expects(averagingCycles_ > 0);
    Expects(averagingEvery_ > 0 && averagingEvery_ < averagingCycles_);

    this->temperature = temperature_;
    this->pressure = pressure_;
    this->thermalisationCycles = thermalisationCycles_;
    this->averagingCycles = averagingCycles_;
    this->averagingEvery = averagingEvery_;
    this->reset();

    this->shouldAdjustStepSize = true;
    logger.setAdditionalText("thermalisation");
    logger.info() << "Starting thermalisation..." << std::endl;
    for (std::size_t i{}; i < this->thermalisationCycles; i++) {
        this->performCycle(logger, interaction);
        if ((i + 1) % this->averagingEvery == 0)
            this->densityThermalisationSnapshots.push_back({i + 1,this->packing->getNumberDensity()});
        if ((i + 1) % 100 == 0) {
            logger.info() << "Performed " << (i + 1) << " cycles. ";
            logger << "Number density: " << this->packing->getNumberDensity() << std::endl;
        }
    }

    this->shouldAdjustStepSize = false;
    logger.setAdditionalText("averaging");
    logger.info() << "Starting averaging..." << std::endl;
    for(std::size_t i{}; i < this->averagingCycles; i++) {
        this->performCycle(logger, interaction);
        if ((i + 1) % this->averagingEvery == 0) {
            this->averagedDensities.push_back(this->packing->getNumberDensity());
            this->averagedEnergy.push_back(this->packing->getTotalEnergy(interaction));
            this->averagedEnergyFluctuations.push_back(this->packing->getParticleEnergyFluctuations(interaction));
        }
        if ((i + 1) % 100 == 0) {
            logger.info() << "Performed " << (i + 1) << " cycles. ";
            logger << "Number density: " << this->packing->getNumberDensity() << std::endl;
        }
    }

    logger.setAdditionalText("");
}

void Simulation::reset() {
    this->particleIdxDistribution = std::uniform_int_distribution<int>(0, this->packing->size() - 1);
    this->moveCounter.reset();
    this->scalingCounter.reset();
    this->averagedDensities.clear();
    this->densityThermalisationSnapshots.clear();
    this->packing->resetCounters();
    this->moveMicroseconds = 0;
    this->scalingMicroseconds = 0;
}

void Simulation::performCycle(Logger &logger, const Interaction &interaction) {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    for (std::size_t i{}; i < this->packing->size(); i++) {
        bool wasMoved = this->tryMove(interaction);
        this->moveCounter.increment(wasMoved);
    }
    auto end = high_resolution_clock::now();
    this->moveMicroseconds += duration<double, std::micro>(end - start).count();

    start = high_resolution_clock::now();
    bool wasScaled = this->tryScaling(interaction);
    this->scalingCounter.increment(wasScaled);
    end = high_resolution_clock::now();
    this->scalingMicroseconds += duration<double, std::micro>(end - start).count();

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
        this->packing->acceptTranslation();
        return true;
    } else {
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
        this->packing->acceptRotation();
        return true;
    } else {
        return false;
    }
}

bool Simulation::tryMove(const Interaction &interaction) {
    Vector<3> translation{2*this->unitIntervalDistribution(this->mt) - 1,
                          2*this->unitIntervalDistribution(this->mt) - 1,
                          2*this->unitIntervalDistribution(this->mt) - 1};
    translation *= this->translationStep;

    Vector<3> axis;
    do {
        axis[0] = 2*this->unitIntervalDistribution(this->mt) - 1;
        axis[1] = 2*this->unitIntervalDistribution(this->mt) - 1;
        axis[2] = 2*this->unitIntervalDistribution(this->mt) - 1;
    } while (axis.norm2() > 1);
    double angle = (2*this->unitIntervalDistribution(this->mt) - 1) * std::min(this->rotationStep, M_PI);
    auto rotation = Matrix<3, 3>::rotation(axis.normalized(), angle);

    double dE = this->packing->tryMove(this->particleIdxDistribution(this->mt), translation, rotation, interaction);
    if (this->unitIntervalDistribution(this->mt) <= std::exp(-dE / this->temperature)) {
        this->packing->acceptMove();
        return true;
    } else {
        return false;
    }
}

bool Simulation::tryScaling(const Interaction &interaction) {
    double deltaV = (2*this->unitIntervalDistribution(this->mt) - 1) * this->scalingStep;
    double currentV = this->packing->getVolume();
    double factor = (deltaV + currentV) / currentV;
    if (factor < 0)
        return false;

    double N = this->packing->size();
    double dE = this->packing->tryScaling(factor, interaction);
    double exponent = N * log(factor) - dE / this->temperature - this->pressure * deltaV / this->temperature;
    if (this->unitIntervalDistribution(this->mt) <= std::exp(exponent)) {
        return true;
    } else {
        this->packing->revertScaling();
        return false;
    }
}

void Simulation::evaluateCounters(Logger &logger) {
    if (this->moveCounter.movesSinceEvaluation >= 100 * this->packing->size()) {
        double rate = this->moveCounter.getCurrentRate();
        this->moveCounter.resetCurrent();
        if (rate > 0.2) {
            const auto &dimensions = this->packing->getDimensions();
            double minDimension = *std::min_element(dimensions.begin(), dimensions.end());
            // Current policy: adjust translations and rotations at the same time - the ratio from the config file
            // is kept. Translation step can be as large as the packing, but not larger. Rotation step would usually
            // be > M_PI then anyway
            if (this->translationStep * 1.1 <= minDimension) {
                this->translationStep *= 1.1;
                this->rotationStep *= 1.1;
                logger.info() << "Translation rate: " << rate << ", adjusting: "  << (this->translationStep / 1.1);
                logger << " -> " << this->translationStep << std::endl;
                logger.info() << "Rotation rate: " << rate << ", adjusting: "  << (this->rotationStep / 1.1);
                logger << " -> " << this->rotationStep << std::endl;
            }
        } else if (rate < 0.1) {
            this->translationStep /= 1.1;
            this->rotationStep /= 1.1;
            logger.info() << "Translation rate: " << rate << ", adjusting: " << (this->translationStep * 1.1);
            logger << " -> " << this->translationStep << std::endl;
            logger.info() << "Rotation rate: " << rate << ", adjusting: " << (this->rotationStep * 1.1);
            logger << " -> " << this->rotationStep << std::endl;
        }
    }

    if (this->scalingCounter.movesSinceEvaluation >= 100) {
        double rate = this->scalingCounter.getCurrentRate();
        this->scalingCounter.resetCurrent();
        if (rate > 0.2) {
            this->scalingStep *= 1.1;
            logger.info() << "Scaling rate: " << rate << ", adjusting: " << (this->scalingStep / 1.1);
            logger << " -> " << this->scalingStep << std::endl;
        } else if (rate < 0.1) {
            this->scalingStep /= 1.1;
            logger.info() << "Scaling rate: " << rate << ", adjusting: " << (this->scalingStep * 1.1);
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
