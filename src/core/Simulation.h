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

/**
 * @brief A class responsible for performing Monte Carlo sampling.
 * @details Actual moves are done by Packing class - this class only check Metropolis criterion and accepts or rejects
 * them. It also does other "higher level" things such as collecting observables, etc.
 */
class Simulation {
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
    double domainDecompositionMicroseconds{};
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
    void printInlineInfo(std::size_t cycleNumber, const ShapeTraits &traits, Logger &logger);

public:
    /**
     * @brief Constructs the simulation for given parameters.
     * @param packing initial configuration of shapes
     * @param translationStep initial translation step size
     * @param rotationStep initial rotation step size
     * @param scalingStep initial volume scaling step
     * @param seed seed of the RNG
     * @param volumeScaler volume move scaling sampler
     * @param domainDivisions domain divisions in each direction to use; {1, 1, 1} disables domain division
     * @param handleSignals if @a true, SIGINT and SIGCONT will be captured
     */
    Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep, double scalingStep,
               unsigned long seed, std::unique_ptr<VolumeScaler> volumeScaler,
               const std::array<std::size_t, 3> &domainDivisions = {1, 1, 1},
               bool handleSignals = false);

    /**
     * @brief Performs the simulation.
     * @param temperature_ temperature of the system
     * @param pressure_ system pressure
     * @param thermalisationCycles_ the number of cycles in thermalization phase
     * @param averagingCycles_ the number of cycles in averaging (production) phase
     * @param averagingEvery_ how often to take observables for averaging
     * @param snapshotEvery_ how often to take observable snapshots
     * @param shapeTraits shape traits describing the simulated molecules
     * @param observablesCollector_ the observables collector with observable capturing configuration
     * @param logger Logger object to display simulation data
     * @param cycleOffset the initial cycle of the simulation (if for example the previous run was disrupted)
     */
    void perform(double temperature_, double pressure_, std::size_t thermalisationCycles_, std::size_t averagingCycles_,
                 std::size_t averagingEvery_, std::size_t snapshotEvery_, const ShapeTraits &shapeTraits,
                 std::unique_ptr<ObservablesCollector> observablesCollector_, Logger &logger,
                 std::size_t cycleOffset = 0);

    [[nodiscard]] const ObservablesCollector &getObservablesCollector() { return *this->observablesCollector; }

    /**
     * @brief Returns the ratio of accepted to all molecule moves.
     */
    [[nodiscard]] double getMoveAcceptanceRate() const { return this->moveCounter.getRate(); }

    /**
     * @brief Returns the ratio of accepted to all scaling moves.
     */
    [[nodiscard]] double getScalingAcceptanceRate() const { return this->scalingCounter.getRate(); }

    /**
     * @brief Returns the total time consumed by molecule moves in microseconds.
     */
    [[nodiscard]] double getMoveMicroseconds() const { return this->moveMicroseconds; }

    /**
     * @brief Returns the total time consumed by scaling moves in microseconds.
     */
    [[nodiscard]] double getScalingMicroseconds() const { return this->scalingMicroseconds; }

    /**
     * @brief Returns the total time consumed by domain decomposition moves in microseconds.
     */
    [[nodiscard]] double getDomainDecompositionMicroseconds() const { return this->domainDecompositionMicroseconds; }

    /**
     * @brief Returns the total time consumed by computation of observables.
     */
    [[nodiscard]] double getObservablesMicroseconds() const {
        return this->observablesCollector->getComputationMicroseconds();
    }

    [[nodiscard]] const Packing &getPacking() const { return *this->packing; }
    [[nodiscard]] double getCurrentTranslationStep() const { return this->translationStep; }
    [[nodiscard]] double getCurrentRotationStep() const { return this->rotationStep; }
    [[nodiscard]] double getCurrentScalingStep() const { return this->scalingStep; }

    /**
     * @brief Returns total number of performed MC cycles (together with cycle offset)
     */
    [[nodiscard]] std::size_t getTotalCycles() const { return this->totalCycles; }

    /**
     * @brief Returns number of cycles actually performed by the class (not counging the cycle offset)
     */
    [[nodiscard]] std::size_t getPerformedCycles() const { return this->performedCycles; }

    /**
     * @brief Returns true if the current run was interrupted by SIGCONT or SIGINT.
     */
    [[nodiscard]] bool wasInterrupted() const;
};


#endif //RAMPACK_SIMULATION_H
