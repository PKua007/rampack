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
#include "ShapeTraits.h"
#include "VolumeScaler.h"
#include "ObservablesCollector.h"

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
        std::size_t movesSinceEvaluation{};
        std::size_t acceptedMovesSinceEvaluation{};
        std::size_t moves{};
        std::size_t acceptedMoves{};

    public:
        void increment(bool accepted);
        void reset();
        void resetCurrent();

        [[nodiscard]] std::size_t getMovesSinceEvaluation() const;
        [[nodiscard]] double getCurrentRate() const;
        [[nodiscard]] double getRate() const;

        Counter &operator+=(const Counter &other);
        friend Counter operator+(Counter c1, const Counter &c2) { return c1 += c2; }
    };

    std::size_t thermalisationCycles{};
    std::size_t averagingCycles{};
    std::size_t averagingEvery{};
    std::size_t snapshotEvery{};

    double temperature{};
    double pressure{};

    double translationStep{};
    double rotationStep{};
    double scalingStep{};
    std::unique_ptr<VolumeScaler> volumeScaler{};
    Counter moveCounter;
    Counter scalingCounter;
    double moveMicroseconds{};
    double scalingMicroseconds{};
    bool shouldAdjustStepSize{};
    std::size_t performedCycles{};
    std::size_t totalCycles{};

    std::vector<std::mt19937> mts;
    std::uniform_real_distribution<double> unitIntervalDistribution;

    std::unique_ptr<Packing> packing;
    std::vector<std::size_t> allParticleIndices;
    std::array<std::size_t, 3> domainDivisions;
    std::size_t numDomains{};

    std::unique_ptr<ObservablesCollector> observablesCollector;

    void performCycle(Logger &logger, const Interaction &interaction);
    void performMovesWithDomainDivision(const Interaction &interaction);
    void performMovesWithoutDomainDivision(const Interaction &interaction);
    bool tryTranslation(const Interaction &interaction, const std::vector<std::size_t> &particleIndices,
                        std::optional<ActiveDomain> boundaries = std::nullopt);
    bool tryRotation(const Interaction &interaction, const std::vector<std::size_t> &particleIndices);
    bool tryMove(const Interaction &interaction, const std::vector<std::size_t> &particleIndices,
                 std::optional<ActiveDomain> boundaries = std::nullopt);
    bool tryScaling(const Interaction &interaction);
    void evaluateCounters(Logger &logger);
    void reset();

public:
    Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep, double scalingStep,
               unsigned long seed, std::unique_ptr<VolumeScaler> volumeScaler,
               const std::array<std::size_t, 3> &domainDivisions = {1, 1, 1},
               bool handleSignals = false);

    void perform(double temperature_, double pressure_, std::size_t thermalisationCycles_, std::size_t averagingCycles_,
                 std::size_t averagingEvery_, std::size_t snapshotEvery_, const ShapeTraits &shapeTraits,
                 std::unique_ptr<ObservablesCollector> observablesCollector_, Logger &logger,
                 std::size_t cycleOffset = 0);
    [[nodiscard]] const ObservablesCollector &getObservablesCollector() { return *this->observablesCollector; }
    [[nodiscard]] double getMoveAcceptanceRate() const { return this->moveCounter.getRate(); }
    [[nodiscard]] double getScalingAcceptanceRate() const { return this->scalingCounter.getRate(); }
    [[nodiscard]] double getMoveMicroseconds() const { return this->moveMicroseconds; }
    [[nodiscard]] double getScalingMicroseconds() const { return this->scalingMicroseconds; }
    [[nodiscard]] double getObservablesMicroseconds() const {
        return this->observablesCollector->getComputationMicroseconds();
    }
    [[nodiscard]] const Packing &getPacking() const { return *this->packing; }
    [[nodiscard]] double getCurrentTranslationStep() const { return this->translationStep; }
    [[nodiscard]] double getCurrentRotationStep() const { return this->rotationStep; }
    [[nodiscard]] double getCurrentScalingStep() const { return this->scalingStep; }
    [[nodiscard]] std::size_t getTotalCycles() const { return this->totalCycles; }
    [[nodiscard]] std::size_t getPerformedCycles() const { return this->performedCycles; }

    void printInlineInfo(std::size_t cycleNumber, const ShapeTraits &traits, Logger &logger);
};


#endif //RAMPACK_SIMULATION_H
