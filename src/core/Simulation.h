//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SIMULATION_H
#define RAMPACK_SIMULATION_H

#include <random>

#include "Packing.h"
#include "utils/Logger.h"
#include "utils/Quantity.h"

class Simulation {
private:
    struct Counter {
        std::size_t movesSinceEvaluation{};
        std::size_t acceptedMovesSinceEvaluation{};
        std::size_t moves{};
        std::size_t acceptedMoves{};

        void increment(bool accepted);
        void reset();
        void resetCurrent();

        [[nodiscard]] double getCurrentRate() const {
            return static_cast<double>(this->acceptedMovesSinceEvaluation) / this->movesSinceEvaluation;
        }

        [[nodiscard]] double getRate() const {
            return static_cast<double>(this->acceptedMoves) / this->moves;
        }
    };

    double temperature{};
    double pressure{};
    std::size_t thermalisationCycles{};
    std::size_t averagingCycles{};
    std::size_t averagingEvery{};
    std::size_t cycleLength{};
    bool shouldAdjustStepSize{};

    std::mt19937 mt;
    std::uniform_real_distribution<double> translationDistribution;
    std::uniform_real_distribution<double> scalingDistribution;
    std::uniform_int_distribution<int> moveTypeDistribution;
    std::uniform_real_distribution<double> unitIntervalDistribution;
    std::uniform_int_distribution<int> particleIdxDistribution;

    std::unique_ptr<Packing> packing;
    std::vector<double> densityShaphots;
    Counter translationCounter;
    Counter scalingCounter;

    void performStep(Logger &logger);
    bool tryScaling();
    bool tryTranslation();
    void evaluateCounters(Logger &logger);

public:
    Simulation(double temperature, double pressure, double positionStepSize, double volumeStepSize,
               std::size_t thermalisationCycles, std::size_t averagingCycles, std::size_t averagingEvery,
               unsigned long seed);

    void perform(std::unique_ptr<Packing> packing_, Logger &logger);
    [[nodiscard]] Quantity getAverageDensity() const;
    [[nodiscard]] double getTranlationAcceptanceRate() const { return this->translationCounter.getRate(); }
    [[nodiscard]] double getScalingAcceptanceRate() const { return this->scalingCounter.getRate(); }
    [[nodiscard]] const Packing &getPacking() const { return *this->packing; }
};


#endif //RAMPACK_SIMULATION_H
