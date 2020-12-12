//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SIMULATION_H
#define RAMPACK_SIMULATION_H

#include <random>

#include "Packing.h"
#include "utils/Logger.h"

class Simulation {
private:
    double temperature{};
    double pressure{};
    std::size_t thermalisationSteps{};
    std::size_t averagingSteps{};

    std::mt19937 mt;
    std::uniform_real_distribution<double> translationDistribution;
    std::uniform_real_distribution<double> scalingDistribution;
    std::uniform_int_distribution<int> moveTypeDistribution;
    std::uniform_real_distribution<double> unitIntervalDistribution;
    std::uniform_int_distribution<int> particleIdxDistribution;

    std::unique_ptr<Packing> packing;
    double densitySum{};
    std::size_t acceptedThermalisationSteps{};
    std::size_t acceptedAveragingSteps{};

    bool performStep();

public:
    Simulation(std::unique_ptr<Packing> packing, double temperature, double pressure, double positionStepSize,
               double volumeStepSize, std::size_t thermalisationsSteps, std::size_t averagingSteps, unsigned long seed);

    void perform(Logger &logger);
    [[nodiscard]] double getAverageDensity() const { return this->densitySum / this->averagingSteps; };
    [[nodiscard]] double getThermalisationAcceptanceRate() const {
        return static_cast<double>(this->acceptedThermalisationSteps) / this->thermalisationSteps;
    }

    [[nodiscard]] double getAveragingAcceptanceRate() const {
        return static_cast<double>(this->acceptedAveragingSteps) / this->averagingSteps;
    }
};


#endif //RAMPACK_SIMULATION_H
