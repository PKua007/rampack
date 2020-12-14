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
    std::size_t evaluateTranslationEvery = 1000;
    std::size_t evaluateScalingEvery = 100;
    double minAcceptanceRatio = 0.3;
    double maxAcceptanceRatio = 0.6;

    std::mt19937 mt;
    std::uniform_real_distribution<double> translationDistribution;
    std::uniform_real_distribution<double> scalingDistribution;
    std::uniform_int_distribution<int> moveTypeDistribution;
    std::uniform_real_distribution<double> unitIntervalDistribution;
    std::uniform_int_distribution<int> particleIdxDistribution;

    std::unique_ptr<Packing> packing;
    double densitySum{};
    std::size_t translationMoves{};
    std::size_t translationMovesSinceEval{};
    std::size_t acceptedTranslations{};
    std::size_t acceptedTranslationsSinceEval{};
    std::size_t scalingMoves{};
    std::size_t scalingMovesSinceEval{};
    std::size_t acceptedScalings{};
    std::size_t acceptedScalingsSinceEval{};

    bool performStep(Logger &logger);

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

    [[nodiscard]] const Packing &getPacking() const { return *this->packing; }
};


#endif //RAMPACK_SIMULATION_H
