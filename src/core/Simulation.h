//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SIMULATION_H
#define RAMPACK_SIMULATION_H


// Uncomment to sanitize whether cached number of overlaps is consistent with a real one after both molecule moves
// and scaling moves

// #define SIMULATION_SANITIZE_CACHED_OVERLAPS

#include <random>
#include <iosfwd>
#include <optional>
#include <utility>

#include "Packing.h"
#include "utils/Logger.h"
#include "utils/Quantity.h"
#include "ShapeTraits.h"
#include "TriclinicBoxScaler.h"
#include "ObservablesCollector.h"
#include "MoveSampler.h"

/**
 * @brief A class responsible for performing Monte Carlo sampling.
 * @details Actual moves are done by Packing class - this class only check Metropolis criterion and accepts or rejects
 * them. It also does other "higher level" things such as collecting observables, etc.
 */
class Simulation {
public:
    /**
     * @brief Helper struct for named step sizes.
     */
    struct StepSizeData {
        /** @brief Name of the move. */
        const std::string moveName;
        /** @brief Step size of the move. */
        const double stepSize{};

        StepSizeData(std::string moveName, double stepSize) : moveName{std::move(moveName)}, stepSize{stepSize} { }
    };

    /**
     * @brief Move statistics for a whole MoveSampler.
     */
    struct MoveStatistics {
        /** @brief Name of the whoile MoveSampler. */
        const std::string groupName{};
        /** @brief Total number of performed moves (both accepted and rejected). */
        std::size_t totalMoves{};
        /** @brief Total number of accepted moves. */
        std::size_t acceptedMoves{};
        /** @brief Vector of step sizes data for all constituent moves of MoveSampler */
        const std::vector<StepSizeData> stepSizeDatas{};

        MoveStatistics(std::string groupName, size_t totalMoves, size_t acceptedMoves,
                       std::vector<StepSizeData> stepSizeDatas)
                : groupName{std::move(groupName)}, totalMoves{totalMoves}, acceptedMoves{acceptedMoves},
                  stepSizeDatas{std::move(stepSizeDatas)}
        { }

        /**
         * @brief Calculates move rate (ratio of accepted to total).
         */
        [[nodiscard]] double getRate() const {
            return static_cast<double>(this->acceptedMoves)
                   / static_cast<double>(this->totalMoves);
        }
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
        [[nodiscard]] std::size_t getMoves() const;
        [[nodiscard]] std::size_t getAcceptedMoves() const;
        [[nodiscard]] double getCurrentRate() const;
        [[nodiscard]] double getRate() const;

        Counter &operator+=(const Counter &other);
        friend Counter operator+(Counter c1, const Counter &c2) { return c1 += c2; }
    };

    double temperature{};
    double pressure{};

    std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
    std::vector<bool> adjustmentCancelReported;

    double scalingStep{};
    std::unique_ptr<TriclinicBoxScaler> boxScaler{};
    std::vector<Counter> moveCounters;
    Counter scalingCounter;
    double moveMicroseconds{};
    double scalingMicroseconds{};
    double domainDecompositionMicroseconds{};
    double totalMicroseconds{};
    bool shouldAdjustStepSize{};
    bool areOverlapsCounted{};
    std::size_t performedCycles{};
    std::size_t totalCycles{};

    std::vector<std::mt19937> mts;
    std::uniform_real_distribution<double> unitIntervalDistribution;

    std::unique_ptr<Packing> packing;
    std::vector<std::size_t> allParticleIndices;
    std::array<std::size_t, 3> domainDivisions;
    std::size_t numDomains{};

    std::unique_ptr<ObservablesCollector> observablesCollector;

    static std::vector<std::unique_ptr<MoveSampler>> makeRototranslation(double translationStepSize,
                                                                         double rotationStepSize);
    static void accumulateCounters(std::vector<Counter> &out, const std::vector<Counter> &in);
    static void printStepSizesChange(Logger &logger, const std::vector<std::pair<std::string, double>> &oldStepSizes,
                                     const std::vector<std::pair<std::string, double>> &newStepSizes);

    void performCycle(Logger &logger, const ShapeTraits &shapeTraits);
    void performMovesWithDomainDivision(const ShapeTraits &shapeTraits);
    void performMovesWithoutDomainDivision(const ShapeTraits &shapeTraits);
    bool tryMove(const ShapeTraits &shapeTraits, const std::vector<std::size_t> &particleIndices,
                 std::vector<Counter> &moveCounters_, const std::vector<std::size_t> &moveTypeAccumulations,
                 std::optional<ActiveDomain> boundaries = std::nullopt);
    bool tryScaling(const Interaction &interaction);
    void evaluateCounters(Logger &logger);
    void reset();
    void printInlineInfo(std::size_t cycleNumber, const ShapeTraits &traits, Logger &logger, bool displayOverlaps);
    [[nodiscard]] std::vector<std::size_t> calculateMoveTypeAccumulations(std::size_t numParticles) const;

    [[nodiscard]] MoveStatistics getScalingStatistics() const;

public:
    /**
     * @brief Constructs the simulation for given parameters - "legacy" version with hardcoded rototranslation moves.
     * @param packing initial configuration of shapes
     * @param translationStep initial translation step size
     * @param rotationStep initial rotation step size
     * @param scalingStep initial volume scaling step
     * @param seed seed of the RNG
     * @param boxScaler volume move scaling sampler
     * @param domainDivisions domain divisions in each direction to use; {1, 1, 1} disables domain division
     * @param handleSignals if @a true, SIGINT and SIGCONT will be captured
     */
    Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep,
               double scalingStep, unsigned long seed, std::unique_ptr<TriclinicBoxScaler> boxScaler,
               const std::array<std::size_t, 3> &domainDivisions = {1, 1, 1}, bool handleSignals = false);

    /**
     * @brief Constructs the simulation for given parameters - "new" version with programmable molecule moves.
     * @param packing initial configuration of shapes
     * @param moveSamplers the list of move samplers to perform molecule moves
     * @param scalingStep initial volume scaling step
     * @param seed seed of the RNG
     * @param boxScaler volume move scaling sampler
     * @param domainDivisions domain divisions in each direction to use; {1, 1, 1} disables domain division
     * @param handleSignals if @a true, SIGINT and SIGCONT will be captured
     */
    Simulation(std::unique_ptr<Packing> packing, std::vector<std::unique_ptr<MoveSampler>> moveSamplers,
               double scalingStep, unsigned long seed, std::unique_ptr<TriclinicBoxScaler> boxScaler,
               const std::array<std::size_t, 3> &domainDivisions = {1, 1, 1}, bool handleSignals = false);

    /**
     * @brief Performs standard Monte Carlo integration consisting of thermalization (equilibration) phase and averaging
     * (production) phase.
     * @param temperature_ temperature of the system
     * @param pressure_ system pressure
     * @param thermalisationCycles the number of cycles in thermalization phase
     * @param averagingCycles the number of cycles in averaging (production) phase
     * @param averagingEvery how often to take observables for averaging
     * @param snapshotEvery how often to take observable snapshots
     * @param shapeTraits shape traits describing the simulated molecules
     * @param observablesCollector_ the observables collector with observable capturing configuration
     * @param logger Logger object to display simulation data
     * @param cycleOffset the initial cycle of the simulation (if for example the previous run was disrupted)
     */
    void integrate(double temperature_, double pressure_, std::size_t thermalisationCycles, std::size_t averagingCycles,
                   std::size_t averagingEvery, std::size_t snapshotEvery, const ShapeTraits &shapeTraits,
                   std::unique_ptr<ObservablesCollector> observablesCollector_, Logger &logger,
                   std::size_t cycleOffset = 0);

    /**
     * @brief Perform overlap reduction scheme - overlap counting is turned on and moves are continued until there is no
     * overlaps in the system.
     * @param temperature_ temperature of the system
     * @param pressure_ system pressure
     * @param snapshotEvery how often to take observable snapshots
     * @param shapeTraits shape traits describing the simulated molecules
     * @param observablesCollector_ the observables collector with observable capturing configuration
     * @param logger  Logger object to display simulation data
     * @param cycleOffset the initial cycle of the simulation (if for example the previous run was disrupted)
     */
    void relaxOverlaps(double temperature_, double pressure_, std::size_t snapshotEvery,
                       const ShapeTraits &shapeTraits, std::unique_ptr<ObservablesCollector> observablesCollector_,
                       Logger &logger, std::size_t cycleOffset = 0);

    [[nodiscard]] const ObservablesCollector &getObservablesCollector() { return *this->observablesCollector; }

    /**
     * @brief Returns move statistics for all MoveSamplers.
     */
    [[nodiscard]] std::vector<MoveStatistics> getMovesStatistics() const;

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

    /**
     * @brief Returns the total time consumed by the simulation.
     */
    [[nodiscard]] double getTotalMicroseconds() const { return this->totalMicroseconds; }

    [[nodiscard]] const Packing &getPacking() const { return *this->packing; }
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
