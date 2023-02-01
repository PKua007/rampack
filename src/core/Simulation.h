//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SIMULATION_H
#define RAMPACK_SIMULATION_H


// Uncomment to sanitize overlaps.
// For overlap reduction sanitize whether cached number of overlaps is consistent with a real one after both molecule
// moves and scaling moves.
// For normal integration, sanitize whether there are no overlaps after molecule moves and after scaling moves

// #define SIMULATION_SANITIZE_OVERLAPS

#include <random>
#include <iosfwd>
#include <optional>
#include <utility>
#include <variant>

#include "Packing.h"
#include "utils/Logger.h"
#include "utils/Quantity.h"
#include "ShapeTraits.h"
#include "TriclinicBoxScaler.h"
#include "ObservablesCollector.h"
#include "MoveSampler.h"
#include "SimulationRecorder.h"
#include "DynamicParameter.h"
#include "DomainDecomposition.h"


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

    /**
     * @brief A helper class holding a DynamicParameter.
     * @brief It is used for arguments of simulation procedures to provide both backwards compatibility (when an
     * ordinary value is passed as a parameter) as well as enable the use of a DynamicParameter. Both types of
     * parameters are implicitly converted into this class, so a user can just skip specifying Parameter type
     * explicitly.
     */
    class Parameter {
    private:
        friend class Simulation;

        std::shared_ptr<DynamicParameter> parameter;

    public:
        Parameter() = default;
        Parameter(double value);
        Parameter(std::shared_ptr<DynamicParameter> updater) : parameter{std::move(updater)} { }
        Parameter(std::unique_ptr<DynamicParameter> updater) : parameter{std::move(updater)} { }
        operator DynamicParameter&() { return *this->parameter; }
        operator const DynamicParameter&() const { return *this->parameter; }
        [[nodiscard]] bool hasValue() const { return this->parameter != nullptr; }
    };

    /**
     * @brief Environment of the simulation, which may be inherited from a previous run and partially of fully
     * overriden.
     * @details <p> It includes temperature, pressure, MoveSampler -s and TriclinicBoxScaler. The inheritability of the
     * last facilitates remembering step sizes from the previous run.
     *
     * <p> Each field is optional and may or may not have a value. By default all values are not set. When combining
     * with another environment (see combine() method), only the values which are set override old values.
     */
    struct Environment {
    private:
        enum class BoxScalerStatus {
            UNSET,
            DISABLED,
            ENEBLED_AND_SET
        };

        Parameter temperature;
        Parameter pressure;
        std::vector<std::shared_ptr<MoveSampler>> moveSamplers;
        std::vector<std::shared_ptr<const MoveSampler>> constMoveSamplers;
        std::shared_ptr<TriclinicBoxScaler> boxScaler;
        BoxScalerStatus boxScalerStatus = BoxScalerStatus::UNSET;

    public:
        [[nodiscard]] bool hasTemperature() const { return this->temperature.hasValue(); }
        [[nodiscard]] const DynamicParameter &getTemperature() const;
        DynamicParameter &getTemperature();
        void setTemperature(Parameter temperature_);

        [[nodiscard]] bool hasPressure() const { return this->pressure.hasValue(); }
        [[nodiscard]] const DynamicParameter &getPressure() const;
        DynamicParameter &getPressure();
        void setPressure(Parameter pressure_);

        [[nodiscard]] bool hasMoveSamplers() const { return !this->moveSamplers.empty(); }
        [[nodiscard]] const std::vector<std::shared_ptr<const MoveSampler>> &getMoveSamplers() const;
        const std::vector<std::shared_ptr<MoveSampler>> &getMoveSamplers();
        void setMoveSamplers(std::vector<std::shared_ptr<MoveSampler>> moveSamplers_);

        [[nodiscard]] bool hasBoxScaler() const { return this->boxScalerStatus != BoxScalerStatus::UNSET; }
        [[nodiscard]] bool isBoxScalingEnabled() const {
            return this->boxScalerStatus == BoxScalerStatus::ENEBLED_AND_SET;
        }
        [[nodiscard]] const TriclinicBoxScaler &getBoxScaler() const;
        TriclinicBoxScaler &getBoxScaler();
        void setBoxScaler(std::shared_ptr<TriclinicBoxScaler> boxScaler_);
        void disableBoxScaling();

        /**
         * @brief Returns @a true, if all fields were set and have a value.
         * @details If box scaling is disabled (Environment::disableBoxScaling()), pressure is ignored - it may be not
         * set and the Environment can still be regarded as complete.
         */
        [[nodiscard]] bool isComplete() const;

        /**
         * @brief Overwrited this environment with @a other, however only those values which are present in @a other
         * are overwritten.
         */
        void combine(Environment &other);
    };

    struct IntegrationParameters {
        std::size_t thermalisationCycles{};
        std::size_t averagingCycles{};
        std::size_t averagingEvery = 100;
        std::size_t snapshotEvery = 100;
        std::size_t inlineInfoEvery = 100;
        std::size_t rotationMatrixFixEvery = 10000;
        std::size_t cycleOffset{};
    };

    struct OverlapRelaxationParameters {
        std::size_t snapshotEvery = 100;
        std::size_t inlineInfoEvery = 100;
        std::size_t rotationMatrixFixEvery = 10000;
        std::size_t cycleOffset{};
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

    Environment environment;
    std::vector<bool> adjustmentCancelReported;
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
    std::size_t maxCycles{};

    std::vector<std::mt19937> mts;
    std::uniform_real_distribution<double> unitIntervalDistribution;

    std::unique_ptr<Packing> packing;
    std::vector<std::size_t> allParticleIndices;
    std::array<std::size_t, 3> domainDivisions;
    std::size_t numDomains{};

    std::shared_ptr<ObservablesCollector> observablesCollector;

    static std::vector<std::unique_ptr<MoveSampler>> makeRototranslation(double translationStepSize,
                                                                         double rotationStepSize);
    static Environment makeEnvironment(std::vector<std::unique_ptr<MoveSampler>> moveSamplers,
                                       std::unique_ptr<TriclinicBoxScaler> boxScaler);
    static void accumulateCounters(std::vector<Counter> &out, const std::vector<Counter> &in);
    static void printStepSizesChange(Logger &logger, const std::vector<std::pair<std::string, double>> &oldStepSizes,
                                     const std::vector<std::pair<std::string, double>> &newStepSizes);

    void updateThermodynamicParameters();
    void performCycle(Logger &logger, const ShapeTraits &shapeTraits);
    void performMoves(const ShapeTraits &shapeTraits, Logger &logger);
    void performMovesWithDomainDivision(const ShapeTraits &shapeTraits);
    void performMovesWithoutDomainDivision(const ShapeTraits &shapeTraits);
    bool tryMove(const ShapeTraits &shapeTraits, const std::vector<std::size_t> &particleIndices,
                 std::vector<Counter> &moveCounters_, const std::vector<std::size_t> &moveTypeAccumulations,
                 std::optional<ActiveDomain> boundaries = std::nullopt);
    bool tryScaling(const Interaction &interaction);
    void evaluateCounters(Logger &logger);
    void evaluateMoleculeMoveCounter(Logger &logger);
    void evaluateScalingMoveCounter(Logger &logger);
    void reset();
    void printInlineInfo(std::size_t cycleNumber, const ShapeTraits &traits, Logger &logger, bool displayOverlaps);
    [[nodiscard]] std::vector<std::size_t> calculateMoveTypeAccumulations(std::size_t numParticles) const;
    void fixRotationMatrices(const Interaction &interaction, Logger &logger);
    static double getRotationMatrixDeviation(const Matrix<3, 3> &rotation);

    [[nodiscard]] MoveStatistics getScalingStatistics() const;

public:
    /**
     * @brief Constructs the simulation for given parameters - "new" version with some initial Simulation::Environment.
     * @param packing initial configuration of shapes
     * @param seed seed of the RNG
     * @param initialEnv initial Simulation::Environment, which may be later combined with another one when performing
     * a run
     * @param domainDivisions domain divisions in each direction to use; {1, 1, 1} disables domain division
     * @param handleSignals if @a true, SIGINT and SIGCONT will be captured
     */
    Simulation(std::unique_ptr<Packing> packing, unsigned long seed, Environment initialEnv,
               const std::array<std::size_t, 3> &domainDivisions = {1, 1, 1}, bool handleSignals = false);

    /**
     * @brief Constructs the simulation for given parameters - "new" version with empty Simulation::Environment (a full
     * one has to be specified in the first run).
     * @param packing initial configuration of shapes
     * @param seed seed of the RNG
     * @param domainDivisions domain divisions in each direction to use; {1, 1, 1} disables domain division
     * @param handleSignals if @a true, SIGINT and SIGCONT will be captured
     */
    Simulation(std::unique_ptr<Packing> packing, unsigned long seed,
               const std::array<std::size_t, 3> &domainDivisions = {1, 1, 1}, bool handleSignals = false);

    /**
     * @brief Constructs the simulation for given parameters - "legacy" version with hardcoded rototranslation moves.
     * @param packing initial configuration of shapes
     * @param translationStep initial translation step size
     * @param rotationStep initial rotation step size
     * @param seed seed of the RNG
     * @param boxScaler volume move scaling sampler
     * @param domainDivisions domain divisions in each direction to use; {1, 1, 1} disables domain division
     * @param handleSignals if @a true, SIGINT and SIGCONT will be captured
     */
    Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep, unsigned long seed,
               std::unique_ptr<TriclinicBoxScaler> boxScaler,
               const std::array<std::size_t, 3> &domainDivisions = {1, 1, 1}, bool handleSignals = false);

    /**
     * @brief Constructs the simulation for given parameters - "legacy" version with programmable molecule moves.
     * @param packing initial configuration of shapes
     * @param moveSamplers the list of move samplers to perform molecule moves
     * @param seed seed of the RNG
     * @param boxScaler volume move scaling sampler
     * @param domainDivisions domain divisions in each direction to use; {1, 1, 1} disables domain division
     * @param handleSignals if @a true, SIGINT and SIGCONT will be captured
     */
    Simulation(std::unique_ptr<Packing> packing, std::vector<std::unique_ptr<MoveSampler>> moveSamplers,
               unsigned long seed, std::unique_ptr<TriclinicBoxScaler> boxScaler,
               const std::array<std::size_t, 3> &domainDivisions = {1, 1, 1}, bool handleSignals = false);

    /**
     * @brief Performs standard Monte Carlo integration consisting of thermalization (equilibration) phase and averaging
     * (production) phase - "legacy" version.
     * @details Calls
     * Simulation::integrate(Environment, std::size_t, std::size_t, std::size_t, std::size_t, const ShapeTraits&, std::unique_ptr<ObservablesCollector>, std::unique_ptr<SimulationRecorder>, Logger&, std::size_t)
     * where @a env is given passed @a temeprature_ and @a pressure_, however MoveSampler -s and TriclinicBoxScaler
     * is inherited from the previous run or from the constructor.
     */
    void integrate(Parameter temperature_, Parameter pressure_, std::size_t thermalisationCycles,
                   std::size_t averagingCycles, std::size_t averagingEvery, std::size_t snapshotEvery,
                   const ShapeTraits &shapeTraits, std::shared_ptr<ObservablesCollector> observablesCollector_,
                   std::vector<std::unique_ptr<SimulationRecorder>> simulationRecorders, Logger &logger,
                   std::size_t cycleOffset = 0);
    /**
     * @brief Performs standard Monte Carlo integration consisting of thermalization (equilibration) phase and averaging
     * (production) phase - "new" version with Simulation::Environment.
     * @param env Simulation::Environment, which will be combined the current simulation environment (from the
     * constructor or the previous run)
     * @param params parameters of the integration
     * @param shapeTraits shape traits describing the simulated molecules
     * @param observablesCollector_ the observables collector with observable capturing configuration
     * @param simulationRecorders a list of SimulationRecorders to record the simulation (with a snapshot every
     * @a snapshotEvery). It may be empty
     * @param logger Logger object to display simulation data
     */
    void integrate(Environment env, const IntegrationParameters &params, const ShapeTraits &shapeTraits,
                   std::shared_ptr<ObservablesCollector> observablesCollector_,
                   std::vector<std::unique_ptr<SimulationRecorder>> simulationRecorders, Logger &logger);

    /**
     * @brief Perform overlap reduction scheme - overlap counting is turned on and moves are continued until there is no
     * overlaps in the system - "legacy" version.
     * @details Calls
     * Simulation::relaxOverlaps(Environment, std::size_t, const ShapeTraits&, std::unique_ptr<ObservablesCollector>, std::unique_ptr<SimulationRecorder>, Logger&, std::size_t)
     * where @a env is given passed @a temeprature_ and @a pressure_, however MoveSampler -s and TriclinicBoxScaler is
     * inherited from the previous run or from the constructor.
     */
    void relaxOverlaps(Parameter temperature_, Parameter pressure_, std::size_t snapshotEvery,
                       const ShapeTraits &shapeTraits, std::shared_ptr<ObservablesCollector> observablesCollector_,
                       std::vector<std::unique_ptr<SimulationRecorder>> simulationRecorders, Logger &logger,
                       std::size_t cycleOffset = 0);

    /**
    * @brief Perform overlap reduction scheme - overlap counting is turned on and moves are continued until there is no
    * overlaps in the system - "new" version with Simulation::Environment.
    * @param env Simulation::Environment, which will be combined the current simulation environment (from the
    * constructor or the previous run)
    * @param params parameters of the overlap relaxation
    * @param shapeTraits shape traits describing the simulated molecules
    * @param observablesCollector_ the observables collector with observable capturing configuration
    * @param simulationRecorders a list of SimulationRecorders to record the simulation (with a snapshot every
     * @a snapshotEvery). It may be empty
    * @param logger Logger object to display simulation data
    */
    void relaxOverlaps(Environment env, const OverlapRelaxationParameters &params, const ShapeTraits &shapeTraits,
                       std::shared_ptr<ObservablesCollector> observablesCollector_,
                       std::vector<std::unique_ptr<SimulationRecorder>> simulationRecorders, Logger &logger);

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


    [[nodiscard]] std::size_t getTotalCycles() const { return this->totalCycles; }

    /**
     * @brief Returns number of cycles actually performed by the class (not counging the cycle offset)
     */
    [[nodiscard]] std::size_t getPerformedCycles() const { return this->performedCycles; }

    /**
     * @brief Returns true if the current run was interrupted by SIGCONT or SIGINT.
     */
    [[nodiscard]] bool wasInterrupted() const;

    /**
     * @brief Returns a current temperature of the system.
     */
    [[nodiscard]] double getCurrentTemperature() const { return this->temperature; }

    /**
     * @brief Returns a current pressure of the system.
     */
    [[nodiscard]] double getCurrentPressure() const { return this->pressure; }
};


#endif //RAMPACK_SIMULATION_H
