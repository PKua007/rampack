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
    this->acceptedTranslations = 0;
    this->scalingMoves = 0;
    this->acceptedScalings = 0;
    this->densitySum = 0;

    // Thermalisation
    logger.info() << "Starting thermalisation..." << std::endl;
    for (std::size_t i{}; i < this->thermalisationSteps; i++) {
        this->performStep();
        if ((i + 1) % 10000 == 0)
            logger.info() << "Performed " << (i + 1) << " steps" << std::endl;
    }

    // Averaging
    logger.info() << "Starting averaging..." << std::endl;
    for(std::size_t i{}; i < this->averagingSteps; i++) {
        this->performStep();
        this->densitySum += this->packing->size() / std::pow(this->packing->getLinearSize(), 3);
        if ((i + 1) % 10000 == 0)
            logger.info() << "Performed " << (i + 1) << " steps" << std::endl;
    }
}

bool Simulation::performStep() {
    if (this->moveTypeDistribution(this->mt) == 0) {
        this->scalingMoves++;

        double deltaV = this->scalingDistribution(this->mt);
        double currentV = std::pow(this->packing->getLinearSize(), 3);
        double factor = (deltaV + currentV) / currentV;
        Assert(factor > 0);

        double N = this->packing->size();
        double exponent = N * std::log(factor) - this->pressure * deltaV / this->temperature;
        if (exponent < 0)
            if (this->unitIntervalDistribution(this->mt) < std::exp(exponent))
                return false;

        if (this->packing->tryScaling(factor)) {
            this->acceptedScalings++;
            return true;
        } else{
            return false;
        }
    } else {
        this->translationMoves++;

        std::array<double, 3> translation{this->translationDistribution(this->mt),
                                          this->translationDistribution(this->mt),
                                          this->translationDistribution(this->mt)};
        if (this->packing->tryTranslation(this->particleIdxDistribution(this->mt), translation)) {
            this->acceptedTranslations++;
            return true;
        } else {
            return false;
        }
    }
}
