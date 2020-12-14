//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>

#include "Simulation.h"
#include "utils/Assertions.h"

Simulation::Simulation(std::unique_ptr<Packing> packing, double temperature, double pressure, double positionStepSize,
                       double volumeStepSize, std::size_t thermalisationsSteps, std::size_t averagingSteps,
                       unsigned long seed)
        : temperature{temperature}, pressure{pressure}, thermalisationSteps{thermalisationsSteps},
          averagingSteps{averagingSteps}, mt(seed), translationDistribution(-positionStepSize, positionStepSize),
          scalingDistribution(-volumeStepSize, volumeStepSize), moveTypeDistribution(0, packing->size()),
          particleIdxDistribution(0, packing->size() - 1), packing{std::move(packing)}
{
    Expects(!this->packing->empty());
    Expects(temperature > 0);
    Expects(pressure > 0);
    Expects(positionStepSize > 0);
    Expects(volumeStepSize > 0);
    Expects(thermalisationsSteps > 0);
    Expects(averagingSteps > 0);
}

void Simulation::perform(Logger &logger) {
    this->translationMoves = 0;
    this->translationMovesSinceEval = 0;
    this->acceptedTranslations = 0;
    this->acceptedTranslationsSinceEval = 0;
    this->scalingMoves = 0;
    this->scalingMovesSinceEval = 0;
    this->acceptedScalings = 0;
    this->acceptedScalingsSinceEval = 0;
    this->densitySum = 0;

    // Thermalisation
    logger.info() << "Starting thermalisation..." << std::endl;
    for (std::size_t i{}; i < this->thermalisationSteps; i++) {
        this->performStep(logger);
        if ((i + 1) % 10000 == 0)
            logger.info() << "Performed " << (i + 1) << " steps" << std::endl;
    }

    // Averaging
    logger.info() << "Starting averaging..." << std::endl;
    for(std::size_t i{}; i < this->averagingSteps; i++) {
        this->performStep(logger);
        this->densitySum += this->packing->size() / std::pow(this->packing->getLinearSize(), 3);
        if ((i + 1) % 10000 == 0)
            logger.info() << "Performed " << (i + 1) << " steps" << std::endl;
    }
}

bool Simulation::performStep(Logger &logger) {
    if (this->moveTypeDistribution(this->mt) == 0) {
        this->scalingMoves++;
        this->scalingMovesSinceEval++;

        double deltaV = this->scalingDistribution(this->mt);
        double currentV = std::pow(this->packing->getLinearSize(), 3);
        double factor = (deltaV + currentV) / currentV;
        Assert(factor > 0);

        double N = this->packing->size();
        double exponent = N * std::log(factor) - this->pressure * deltaV / this->temperature;
        bool wasScaled = true;
        if (exponent < 0)
            if (this->unitIntervalDistribution(this->mt) < std::exp(exponent))
                wasScaled = false;

        if (wasScaled)
            wasScaled = this->packing->tryScaling(factor);
        if (wasScaled) {
            this->acceptedScalings++;
            this->acceptedScalingsSinceEval++;
        }
        if (this->scalingMovesSinceEval == this->evaluateScalingEvery) {
            double rate = static_cast<double>(this->acceptedScalingsSinceEval) / this->scalingMovesSinceEval;
            double dV = this->scalingDistribution.param().b();
            if (rate < this->minAcceptanceRatio) {
                logger << "Scaling acceptance rate: " << rate << ", adjusting: " << dV << " -> " << dV / 1.1 << std::endl;
                this->scalingDistribution = std::uniform_real_distribution<double>(-dV / 1.1, dV / 1.1);
            } else if (rate > this->maxAcceptanceRatio) {
                logger << "Scaling acceptance rate: " << rate << ", adjusting: " << dV << " -> " << dV * 1.1 << std::endl;
                this->scalingDistribution = std::uniform_real_distribution<double>(-dV * 1.1, dV * 1.1);
            }
            this->scalingMovesSinceEval = 0;
            this->acceptedScalingsSinceEval = 0;
        }

        return wasScaled;
    } else {
        this->translationMoves++;
        this->translationMovesSinceEval++;

        std::array<double, 3> translation{this->translationDistribution(this->mt),
                                          this->translationDistribution(this->mt),
                                          this->translationDistribution(this->mt)};

        bool wasTranslated = this->packing->tryTranslation(this->particleIdxDistribution(this->mt), translation);
        if (wasTranslated) {
            this->acceptedTranslations++;
            this->acceptedTranslationsSinceEval++;
        }
        if (this->translationMovesSinceEval == this->evaluateTranslationEvery) {
            double rate = static_cast<double>(this->acceptedTranslationsSinceEval) / this->translationMovesSinceEval;
            double dx = this->translationDistribution.param().b();
            if (rate < this->minAcceptanceRatio) {
                logger << "Translation acceptance rate: " << rate << ", adjusting: " << dx << " -> " << dx / 1.1 << std::endl;
                this->translationDistribution = std::uniform_real_distribution<double>(-dx / 1.1, dx / 1.1);
            } else if (rate > this->maxAcceptanceRatio) {
                logger << "Translation acceptance rate: " << rate << ", adjusting: " << dx << " -> " << dx * 1.1 << std::endl;
                this->translationDistribution = std::uniform_real_distribution<double>(-dx * 1.1, dx * 1.1);
            }
            this->translationMovesSinceEval = 0;
            this->acceptedTranslationsSinceEval = 0;
        }
        return wasTranslated;
    }
}
