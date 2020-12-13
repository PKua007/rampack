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
    std::size_t translationMoves{};
    std::size_t acceptedTranslations{};
    std::size_t scalingMoves{};
    std::size_t acceptedScalings{};

    bool performStep();

public:
    Simulation(std::unique_ptr<Packing> packing, double temperature, double pressure, double positionStepSize,
               double volumeStepSize, std::size_t thermalisationsSteps, std::size_t averagingSteps, unsigned long seed);

    void perform(Logger &logger);
    [[nodiscard]] double getAverageDensity() const { return this->densitySum / this->averagingSteps; };
    [[nodiscard]] double getTranlationAcceptanceRate() const {
        return static_cast<double>(this->acceptedTranslations) / static_cast<double>(translationMoves);
    }

    [[nodiscard]] double getScalingAcceptanceRate() const {
        return static_cast<double>(this->acceptedScalings) / static_cast<double>(scalingMoves);
    }
};


#endif //RAMPACK_SIMULATION_H
