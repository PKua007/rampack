//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SIMULATION_H
#define RAMPACK_SIMULATION_H

#include <random>
#include <iosfwd>
#include <optional>

#include "Packing.h"
#include "utils/Logger.h"
#include "utils/Quantity.h"
#include "Interaction.h"

class Simulation {
public:
    struct ScalarSnapshot {
        std::size_t cycleCount{};
        double value{};

        friend std::ostream &operator<<(std::ostream &out, const Simulation::ScalarSnapshot &snapshot);
    };

private:
    class Counter {
    private:
        std::vector<std::size_t> movesSinceEvaluation;
        std::vector<std::size_t> acceptedMovesSinceEvaluation{};
        std::vector<std::size_t> moves{};
        std::vector<std::size_t> acceptedMoves{};

        static std::size_t total(const std::vector<std::size_t> &vec);

    public:
        Counter();

        void setNumThreads(std::size_t numThreads);
        void increment(bool accepted);
        void reset();
        void resetCurrent();

        [[nodiscard]] std::size_t getMovesSinceEvaluation() const;
        [[nodiscard]] double getCurrentRate() const;
        [[nodiscard]] double getRate() const;
    };

    std::size_t thermalisationCycles{};
    std::size_t averagingCycles{};
    std::size_t averagingEvery{};

    double temperature{};
    double pressure{};

    double translationStep{};
    double rotationStep{};
    double scalingStep{};
    Counter moveCounter;
    Counter scalingCounter;
    bool shouldAdjustStepSize{};

    std::vector<std::mt19937> mts;
    std::uniform_real_distribution<double> unitIntervalDistribution;
    std::uniform_int_distribution<int> particleIdxDistribution;

    double moveMicroseconds{};
    double scalingMicroseconds{};

    std::unique_ptr<Packing> packing;
    std::vector<std::size_t> particles;
    std::size_t numDomains{};
    std::array<std::size_t, 3> domainDivisions;

    std::vector<double> averagedDensities;
    std::vector<double> averagedEnergy;
    std::vector<double> averagedEnergyFluctuations;
    std::vector<ScalarSnapshot> densityThermalisationSnapshots;

    void performCycle(Logger &logger, const Interaction &interaction);
    bool tryTranslation(const Interaction &interaction, const std::vector<std::size_t> &particles,
                        std::optional<std::array<std::pair<double, double>, 3>> boundaries = std::nullopt);
    bool tryRotation(const Interaction &interaction, const std::vector<std::size_t> &particles);
    bool tryMove(const Interaction &interaction, const std::vector<std::size_t> &particles,
                 std::optional<std::array<std::pair<double, double>, 3>> boundaries = std::nullopt);
    bool tryScaling(const Interaction &interaction);
    void evaluateCounters(Logger &logger);
    void reset();

public:
    Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep,
               double scalingStep, unsigned long seed, const std::array<std::size_t, 3> &domainDivisions = {1, 1, 1});

    void perform(double temperature_, double pressure_, std::size_t thermalisationCycles_, std::size_t averagingCycles_,
                 std::size_t averagingEvery_, const Interaction &interaction, Logger &logger);
    [[nodiscard]] Quantity getAverageDensity() const;
    [[nodiscard]] Quantity getAverageEnergy() const;
    [[nodiscard]] Quantity getAverageEnergyFluctuations() const;
    [[nodiscard]] std::vector<ScalarSnapshot> getDensityThermalisationSnapshots() const {
        return this->densityThermalisationSnapshots;
    }
    [[nodiscard]] double getMoveAcceptanceRate() const { return this->moveCounter.getRate(); }
    [[nodiscard]] double getScalingAcceptanceRate() const { return this->scalingCounter.getRate(); }
    [[nodiscard]] double getMoveMicroseconds() const { return this->moveMicroseconds; }
    [[nodiscard]] double getScalingMicroseconds() const { return this->scalingMicroseconds; }
    [[nodiscard]] const Packing &getPacking() const { return *this->packing; }
};


#endif //RAMPACK_SIMULATION_H
