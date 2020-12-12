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
          scalingDistribution(-std::log(volumeStepSize), std::log(volumeStepSize)),
          moveTypeDistribution(0, packing->size()), particleIdxDistribution(0, packing->size() - 1),
          packing{std::move(packing)}
{
    Expects(!this->packing->empty());
    Expects(temperature > 0);
    Expects(pressure > 0);
    Expects(positionStepSize > 0);
    Expects(volumeStepSize > 0);
    Expects(thermalisationsSteps > 0);
    Expects(averagingSteps > 0);
}

void Simulation::perform() {
    this->acceptedThermalisationSteps = 0;
    this->acceptedAveragingSteps = 0;
    this->densitySum = 0;

    // Thermalisation
    for (std::size_t i{}; i < this->thermalisationSteps; i++) {
        if (this->performStep())
            this->acceptedThermalisationSteps++;
    }

    // Averaging
    for(std::size_t i{}; i < this->averagingSteps; i++) {
        if (this->performStep())
            this->acceptedAveragingSteps++;
        this->densitySum += this->packing->size() / std::pow(this->packing->getLinearSize(), 3);
    }
}

bool Simulation::performStep() {
    if (this->moveTypeDistribution(this->mt) == 0) {
        // Volume scaling move - on average that many times more rare than translation move as the number of particles
        // in the packing
        double deltaV = std::exp(this->scalingDistribution(this->mt));
        double currentV = std::pow(this->packing->getLinearSize(), 3);
        double factor = (deltaV + currentV) / currentV;
        Assert(factor > 0);

        double N = this->packing->size();
        double exponent = (N + 1) * std::log(factor) - this->pressure * deltaV / this->temperature;
        if (exponent < 0)
            if (this->unitIntervalDistribution(this->mt) < std::exp(exponent))
                return false;

        return this->packing->tryScaling(factor);
    } else {
        // Translation move
        std::array<double, 3> translation{this->translationDistribution(this->mt),
                                          this->translationDistribution(this->mt),
                                          this->translationDistribution(this->mt)};
        return this->packing->tryTranslation(this->particleIdxDistribution(this->mt), translation);
    }
}
