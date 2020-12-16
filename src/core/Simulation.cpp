//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <ostream>

#include "Simulation.h"
#include "utils/Assertions.h"

Simulation::Simulation(double temperature, double pressure, double positionStepSize, double volumeStepSize,
                       std::size_t thermalisationCycles, std::size_t averagingCycles, std::size_t averagingEvery,
                       unsigned long seed)
        : temperature{temperature}, pressure{pressure}, thermalisationCycles{thermalisationCycles},
          averagingCycles{averagingCycles}, averagingEvery{averagingEvery}, mt(seed),
          translationDistribution(-positionStepSize, positionStepSize),
          scalingDistribution(-volumeStepSize, volumeStepSize)
{
    Expects(temperature > 0);
    Expects(pressure > 0);
    Expects(positionStepSize > 0);
    Expects(volumeStepSize > 0);
    Expects(thermalisationCycles > 0);
    Expects(averagingCycles > 0);
    Expects(averagingEvery > 0 && averagingEvery < averagingCycles);
}

void Simulation::perform(std::unique_ptr<Packing> packing_, Logger &logger) {
    Expects(!packing_->empty());
    this->moveTypeDistribution = std::uniform_int_distribution<int>(0, packing_->size());
    this->particleIdxDistribution = std::uniform_int_distribution<int>(0, packing_->size() - 1);
    this->packing = std::move(packing_);
    this->translationCounter.reset();
    this->scalingCounter.reset();
    this->averagedDensities.clear();
    this->densityThermalisationSnapshots.clear();
    this->cycleLength = this->packing->size() + 1;  // size() translations + 1 scaling

    this->shouldAdjustStepSize = true;
    logger.setAdditionalText("thermalisation");
    logger.info() << "Starting thermalisation..." << std::endl;
    for (std::size_t i{}; i < this->thermalisationCycles * this->cycleLength; i++) {
        this->performStep(logger);
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
        this->performStep(logger);
        if ((i + 1) % (this->cycleLength * this->averagingEvery) == 0)
            this->averagedDensities.push_back(this->packing->getNumberDensity());
        if ((i + 1) % (this->cycleLength * 100) == 0)
            logger.info() << "Performed " << ((i + 1)/this->cycleLength) << " cycles" << std::endl;
    }

    logger.setAdditionalText("");
}

void Simulation::performStep(Logger &logger) {
    if (this->moveTypeDistribution(this->mt) == 0) {
       bool wasScaled = this->tryScaling();
       this->scalingCounter.increment(wasScaled);
    } else {
        bool wasTranslated = tryTranslation();
        this->translationCounter.increment(wasTranslated);
    }

    if (this->shouldAdjustStepSize)
        this->evaluateCounters(logger);
}

bool Simulation::tryTranslation() {
    Vector<3> translation{{this->translationDistribution(this->mt),
                           this->translationDistribution(this->mt),
                           this->translationDistribution(this->mt)}};

    return this->packing->tryTranslation(this->particleIdxDistribution(this->mt), translation);
}

bool Simulation::tryScaling() {
    double deltaV = this->scalingDistribution(this->mt);
    double currentV = std::pow(this->packing->getLinearSize(), 3);
    double factor = (deltaV + currentV) / currentV;
    Assert(factor > 0);

    double N = this->packing->size();
    double exponent = N * log(factor) - this->pressure * deltaV / this->temperature;
    if (exponent < 0)
        if (this->unitIntervalDistribution(this->mt) > exp(exponent))
            return false;

    return this->packing->tryScaling(factor);
}

void Simulation::evaluateCounters(Logger &logger) {
    if (this->translationCounter.movesSinceEvaluation >= 100*this->cycleLength) {
        double dx = this->translationDistribution.b();
        double rate = this->translationCounter.getCurrentRate();
        this->translationCounter.resetCurrent();
        if (rate > 0.5) {
            dx *= 1.1;
            if (dx <= this->packing->getLinearSize()) {
                this->translationDistribution = std::uniform_real_distribution<double>(-dx, dx);
                logger.verbose();
                logger << "Translation rate: " << rate << ", adjusting: "  << (dx / 1.1) << " -> " << dx << std::endl;
            }
        } else if (rate < 0.3) {
            dx /= 1.1;
            this->translationDistribution = std::uniform_real_distribution<double>(-dx, dx);
            logger.verbose();
            logger << "Translation rate: " << rate << ", adjusting: " << (dx * 1.1) << " -> " << dx << std::endl;
        }
    }

    if (this->scalingCounter.movesSinceEvaluation >= 100) {
        double dV = this->scalingDistribution.b();
        double rate = this->scalingCounter.getCurrentRate();
        this->scalingCounter.resetCurrent();
        if (rate > 0.5) {
            dV *= 1.1;
            this->scalingDistribution = std::uniform_real_distribution<double>(-dV, dV);
            logger.verbose() << "Scaling rate: " << rate << ", adjusting: " << (dV / 1.1) << " -> " << dV << std::endl;
        } else if (rate < 0.3) {
            dV /= 1.1;
            this->scalingDistribution = std::uniform_real_distribution<double>(-dV, dV);
            logger.verbose() << "Scaling rate: " << rate << ", adjusting: " << (dV * 1.1) << " -> " << dV << std::endl;
        }
    }
}

Quantity Simulation::getAverageDensity() const {
    Quantity density;
    density.separator = Quantity::PLUS_MINUS;
    density.calculateFromSamples(this->averagedDensities);
    return density;
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
