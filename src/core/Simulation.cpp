//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <ostream>
#include <chrono>
#include <atomic>
#include <csignal>

#include "Simulation.h"
#include "DomainDecomposition.h"
#include "utils/Assertions.h"
#include "move_samplers/RototranslationSampler.h"
#include "dynamic_parameters/ConstantDynamicParameter.h"

namespace {
    std::atomic<bool> sigint_received = false;
    static_assert(decltype(sigint_received)::is_always_lock_free);

    class LoggerAdditionalTextAppender {
    private:
        Logger &logger;
        std::string previousAdditionalText;

    public:
        explicit LoggerAdditionalTextAppender(Logger &logger)
                : logger{logger}, previousAdditionalText{logger.getAdditionalText()}
        { }

        ~LoggerAdditionalTextAppender() { this->logger.setAdditionalText(this->previousAdditionalText); }

        void setAdditionalText(const std::string &additionalText) {
            if (this->previousAdditionalText.empty())
                this->logger.setAdditionalText(additionalText);
            else
                this->logger.setAdditionalText(this->previousAdditionalText + ", " + additionalText);
        }
    };
}

void sigint_handler([[maybe_unused]] int signal) {
    sigint_received = true;
}

Simulation::Parameter::Parameter(double value) : parameter{std::make_shared<ConstantDynamicParameter>(value)} {

}

Simulation::Simulation(std::unique_ptr<Packing> packing, std::vector<std::unique_ptr<MoveSampler>> moveSamplers,
                       unsigned long seed, std::unique_ptr<TriclinicBoxScaler> boxScaler,
                       const std::array<std::size_t, 3> &domainDivisions, bool handleSignals)
        : moveSamplers{std::move(moveSamplers)}, boxScaler{std::move(boxScaler)}, packing{std::move(packing)},
          allParticleIndices(this->packing->size()), domainDivisions{domainDivisions}
{
    Expects(!this->packing->empty());
    Expects(!this->moveSamplers.empty());

    this->numDomains = std::accumulate(domainDivisions.begin(), domainDivisions.end(), 1, std::multiplies<>{});
    Expects(this->numDomains > 0);
    Expects(this->numDomains <= this->packing->getMoveThreads());

    this->mts.reserve(this->numDomains);
    for (std::size_t i{}; i < this->numDomains; i++)
        this->mts.emplace_back(seed + i);

    this->moveCounters.resize(this->moveSamplers.size());
    this->adjustmentCancelReported.resize(this->moveSamplers.size(), false);

    std::iota(this->allParticleIndices.begin(), this->allParticleIndices.end(), 0);

    if (handleSignals) {
        std::signal(SIGINT, sigint_handler);
        std::signal(SIGTERM, sigint_handler);
    }
}

Simulation::Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep,
                       unsigned long seed, std::unique_ptr<TriclinicBoxScaler> boxScaler,
                       const std::array<std::size_t, 3> &domainDivisions, bool handleSignals)
        : Simulation(std::move(packing), Simulation::makeRototranslation(translationStep, rotationStep), seed,
                     std::move(boxScaler), domainDivisions, handleSignals)
{ }

void Simulation::integrate(Parameter temperature_, Parameter pressure_, std::size_t thermalisationCycles,
                           std::size_t averagingCycles, std::size_t averagingEvery, std::size_t snapshotEvery,
                           const ShapeTraits &shapeTraits, std::unique_ptr<ObservablesCollector> observablesCollector_,
                           std::unique_ptr<SimulationRecorder> simulationRecorder, Logger &logger,
                           std::size_t cycleOffset)
{
    if (averagingCycles > 0)
        Expects(averagingEvery > 0 && averagingEvery < averagingCycles);

    std::size_t maxCycles = cycleOffset + thermalisationCycles;
    this->temperature = temperature_.parameter->getValueForCycle(cycleOffset, maxCycles);
    this->pressure = pressure_.parameter->getValueForCycle(cycleOffset, maxCycles);
    this->observablesCollector = std::move(observablesCollector_);
    this->observablesCollector->setThermodynamicParameters(this->temperature, this->pressure);
    this->reset();

    this->totalCycles = cycleOffset;

    const Interaction &interaction = shapeTraits.getInteraction();
    LoggerAdditionalTextAppender loggerAdditionalTextAppender(logger);

    auto start = std::chrono::high_resolution_clock::now();

    this->packing->setupForInteraction(interaction);
    this->packing->toggleOverlapCounting(false, interaction);
    this->areOverlapsCounted = false;

    ValidateMsg(this->packing->countTotalOverlaps(interaction) == 0,
                "Overlaps are present at the start of integration. Perform overlap reduction beforehand.");

    this->shouldAdjustStepSize = true;
    loggerAdditionalTextAppender.setAdditionalText("th");
    if (thermalisationCycles == 0) {
        logger.info() << "Thermalization skipped." << std::endl;
    } else {
        logger.info() << "Starting thermalisation..." << std::endl;
        for (std::size_t i{}; i < thermalisationCycles; i++) {
            this->performCycle(logger, shapeTraits);
            this->temperature = temperature_.parameter->getValueForCycle(this->totalCycles, maxCycles);
            this->pressure = pressure_.parameter->getValueForCycle(this->totalCycles, maxCycles);
            this->observablesCollector->setThermodynamicParameters(this->temperature, this->pressure);

            if (this->totalCycles % 10000 == 0)
                this->fixRotationMatrices(shapeTraits.getInteraction(), logger);
            if (this->totalCycles % snapshotEvery == 0) {
                this->observablesCollector->addSnapshot(*this->packing, this->totalCycles, shapeTraits);
                if (simulationRecorder != nullptr)
                    simulationRecorder->recordSnapshot(*this->packing, this->totalCycles);
            }
            if (this->totalCycles % 100 == 0)
                this->printInlineInfo(this->totalCycles, shapeTraits, logger, false);

            if (sigint_received) {
                auto end = std::chrono::high_resolution_clock::now();
                this->totalMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
                logger.warn() << "SIGINT/SIGKILL received, stopping on " << this->totalCycles << " cycle." << std::endl;
                return;
            }
        }
    }

    this->shouldAdjustStepSize = false;
    loggerAdditionalTextAppender.setAdditionalText("av");
    if (averagingCycles == 0) {
        logger.info() << "Averaging skipped." << std::endl;
    } else {
        logger.info() << "Starting averaging..." << std::endl;
        for (std::size_t i{}; i < averagingCycles; i++) {
            this->performCycle(logger, shapeTraits);
            if (this->totalCycles % 10000 == 0)
                this->fixRotationMatrices(shapeTraits.getInteraction(), logger);
            if (this->totalCycles % snapshotEvery == 0) {
                this->observablesCollector->addSnapshot(*this->packing, this->totalCycles, shapeTraits);
                if (simulationRecorder != nullptr)
                    simulationRecorder->recordSnapshot(*this->packing, this->totalCycles);
            }
            if (this->totalCycles % averagingEvery == 0)
                this->observablesCollector->addAveragingValues(*this->packing, shapeTraits);
            if (this->totalCycles % 100 == 0)
                this->printInlineInfo(this->totalCycles, shapeTraits, logger, false);

            if (sigint_received) {
                auto end = std::chrono::high_resolution_clock::now();
                this->totalMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
                logger.warn() << "SIGINT/SIGKILL received, stopping on " << this->totalCycles << " cycle." << std::endl;
                return;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    this->totalMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();

    logger.info() << "Integration completed after " << this->totalCycles << " cycles." << std::endl;
}

void Simulation::relaxOverlaps(Parameter temperature_, Parameter pressure_, std::size_t snapshotEvery,
                               const ShapeTraits &shapeTraits,
                               std::unique_ptr<ObservablesCollector> observablesCollector_,
                               std::unique_ptr<SimulationRecorder> simulationRecorder, Logger &logger,
                               std::size_t cycleOffset)
{
    std::size_t maxCycles = std::numeric_limits<std::size_t>::max();
    this->temperature = temperature_.parameter->getValueForCycle(cycleOffset, maxCycles);
    this->pressure = pressure_.parameter->getValueForCycle(cycleOffset, maxCycles);
    this->observablesCollector = std::move(observablesCollector_);
    this->observablesCollector->setThermodynamicParameters(this->temperature, this->pressure);
    this->reset();

    this->totalCycles = cycleOffset;

    const Interaction &interaction = shapeTraits.getInteraction();
    LoggerAdditionalTextAppender loggerAdditionalTextAppender(logger);

    auto start = std::chrono::high_resolution_clock::now();

    this->packing->setupForInteraction(interaction);
    this->packing->toggleOverlapCounting(true, interaction);
    this->areOverlapsCounted = true;

    this->shouldAdjustStepSize = true;
    loggerAdditionalTextAppender.setAdditionalText("ov");
    logger.info() << "Starting overlap relaxation..." << std::endl;
    while (this->packing->getCachedNumberOfOverlaps() > 0) {
        this->performCycle(logger, shapeTraits);
        this->temperature = temperature_.parameter->getValueForCycle(this->totalCycles, maxCycles);
        this->pressure = pressure_.parameter->getValueForCycle(this->totalCycles, maxCycles);
        this->observablesCollector->setThermodynamicParameters(this->temperature, this->pressure);

        if (this->totalCycles % 10000 == 0)
            this->fixRotationMatrices(shapeTraits.getInteraction(), logger);
        if (this->totalCycles % snapshotEvery == 0) {
            this->observablesCollector->addSnapshot(*this->packing, this->totalCycles, shapeTraits);
            if (simulationRecorder != nullptr)
                simulationRecorder->recordSnapshot(*this->packing, this->totalCycles);
        }
        if (this->totalCycles % 100 == 0)
            this->printInlineInfo(this->totalCycles, shapeTraits, logger, true);

        if (sigint_received) {
            auto end = std::chrono::high_resolution_clock::now();
            this->totalMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();
            logger.warn() << "SIGINT/SIGKILL received, stopping on " << this->totalCycles << " cycle." << std::endl;
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    this->totalMicroseconds = std::chrono::duration<double, std::micro>(end - start).count();

    if (!sigint_received)
        logger.info() << "Overlaps eliminated after " << this->totalCycles << " cycles." << std::endl;
}

void Simulation::reset() {
    std::uniform_int_distribution<int>(0, this->packing->size() - 1);
    for (auto &moveCounter : this->moveCounters)
        moveCounter.reset();
    this->scalingCounter.reset();
    std::fill(this->adjustmentCancelReported.begin(), this->adjustmentCancelReported.end(), false);
    this->packing->resetCounters();
    this->moveMicroseconds = 0;
    this->scalingMicroseconds = 0;
    this->domainDecompositionMicroseconds = 0;
    this->totalMicroseconds = 0;
    this->observablesCollector->clear();
    this->performedCycles = 0;
    this->totalCycles = 0;
    sigint_received = false;
}

void Simulation::performCycle(Logger &logger, const ShapeTraits &shapeTraits) {
    const auto &interaction = shapeTraits.getInteraction();

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    if (this->numDomains == 1)
        this->performMovesWithoutDomainDivision(shapeTraits);
    else
        this->performMovesWithDomainDivision(shapeTraits);
    auto end = high_resolution_clock::now();
    this->moveMicroseconds += duration<double, std::micro>(end - start).count();

    #ifdef SIMULATION_SANITIZE_OVERLAPS
        if (this->areOverlapsCounted) {
            Assert(this->packing->getCachedNumberOfOverlaps() == this->packing->countTotalOverlaps(interaction, false));
        } else {
            Assert(this->packing->countTotalOverlaps(interaction, true) == 0);
        }
    #endif

    start = high_resolution_clock::now();
    bool wasScaled = this->tryScaling(interaction);
    this->scalingCounter.increment(wasScaled);
    end = high_resolution_clock::now();
    this->scalingMicroseconds += duration<double, std::micro>(end - start).count();

    #ifdef SIMULATION_SANITIZE_OVERLAPS
        if (this->areOverlapsCounted) {
            Assert(this->packing->getCachedNumberOfOverlaps() == this->packing->countTotalOverlaps(interaction, false));
        } else {
            Assert(this->packing->countTotalOverlaps(interaction, true) == 0);
        }
    #endif

    if (this->shouldAdjustStepSize)
        this->evaluateCounters(logger);

    this->performedCycles++;
    this->totalCycles++;
}

void Simulation::performMovesWithoutDomainDivision(const ShapeTraits &shapeTraits) {
    auto moveTypeAccumulations = this->calculateMoveTypeAccumulations(this->packing->size());
    std::size_t numMoves = moveTypeAccumulations.back();
    for (std::size_t i{}; i < numMoves; i++)
        this->tryMove(shapeTraits, this->allParticleIndices, this->moveCounters, moveTypeAccumulations);
}

void Simulation::performMovesWithDomainDivision(const ShapeTraits &shapeTraits) {
    const auto &packingBox = this->packing->getBox();
    auto &mt = this->mts[_OMP_THREAD_ID];
    const auto &interaction = shapeTraits.getInteraction();

    Vector<3> randomOrigin{this->unitIntervalDistribution(mt),
                           this->unitIntervalDistribution(mt),
                           this->unitIntervalDistribution(mt)};
    randomOrigin = packingBox.relativeToAbsolute(randomOrigin);
    const auto &neighbourGridCellDivisions = this->packing->getNeighbourGridCellDivisions();
    std::vector<Counter> tempMoveCounters(this->moveCounters.size());

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    DomainDecomposition domainDecomposition(*this->packing, interaction, this->domainDivisions,
                                            neighbourGridCellDivisions, randomOrigin);
    auto end = high_resolution_clock::now();
    this->domainDecompositionMicroseconds += duration<double, std::micro>(end - start).count();

    this->packing->resetNGRaceConditionSanitizer();

    #pragma omp declare reduction (+ : std::vector<Counter> : Simulation::accumulateCounters(omp_out, omp_in)) \
            initializer(omp_priv = omp_orig)
    #pragma omp parallel for shared(domainDecomposition, shapeTraits) default(none) collapse(3) \
            reduction(+ : tempMoveCounters) num_threads(this->packing->getMoveThreads())
    for (std::size_t i = 0; i < this->domainDivisions[0]; i++) {
        for (std::size_t j = 0; j < this->domainDivisions[1]; j++) {
            for (std::size_t k = 0; k < this->domainDivisions[2]; k++) {
                std::array<std::size_t, 3> coords = {i, j, k};

                const auto &domainParticleIndices = domainDecomposition.getParticlesInRegion(coords);
                auto activeDomain = domainDecomposition.getActiveDomainBounds(coords);
                if (domainParticleIndices.empty())
                    continue;

                std::size_t averageNumParticles = this->packing->size() / this->numDomains;
                auto moveTypeAccumulations = this->calculateMoveTypeAccumulations(averageNumParticles);
                std::size_t numMoves = moveTypeAccumulations.back();
                for (std::size_t x{}; x < numMoves; x++) {
                    this->tryMove(shapeTraits, domainParticleIndices, tempMoveCounters, moveTypeAccumulations,
                                  activeDomain);
                }
            }
        }
    }

    this->packing->resetNGRaceConditionSanitizer();

    Simulation::accumulateCounters(this->moveCounters, tempMoveCounters);
}

bool Simulation::tryMove(const ShapeTraits &shapeTraits, const std::vector<std::size_t> &particleIndices,
                         std::vector<Counter> &moveCounters_, const std::vector<std::size_t> &moveTypeAccumulations,
                         std::optional<ActiveDomain> boundaries)
{
    Expects(moveCounters_.size() == this->moveSamplers.size());
    Expects(moveTypeAccumulations.size() == this->moveSamplers.size());

    std::size_t numMoves = moveTypeAccumulations.back();
    std::uniform_int_distribution<std::size_t> moveDistribution(0, numMoves - 1);
    auto &mt = this->mts[_OMP_THREAD_ID];
    std::size_t sampledMoveType = moveDistribution(mt);
    std::size_t moveType{};
    for (auto moveTypeAccumulation : moveTypeAccumulations) {
        if (sampledMoveType < moveTypeAccumulation)
            break;
        moveType++;
    }

    auto &moveSampler = this->moveSamplers[moveType];
    auto move = moveSampler->sampleMove(*this->packing, shapeTraits, particleIndices, mt);
    const auto &interaction = shapeTraits.getInteraction();
    double dE{};
    switch (move.moveType) {
        case MoveSampler::MoveType::TRANSLATION:
            dE = this->packing->tryTranslation(move.particleIdx, move.translation, interaction, boundaries);
            break;
        case MoveSampler::MoveType::ROTATION:
            dE = this->packing->tryRotation(move.particleIdx, move.rotation, interaction);
            break;
        case MoveSampler::MoveType::ROTOTRANSLATION:
            dE = this->packing->tryMove(move.particleIdx, move.translation, move.rotation, interaction, boundaries);
            break;
    }

    auto &moveCounter = moveCounters_[moveType];
    if (this->unitIntervalDistribution(mt) <= std::exp(-dE / this->temperature)) {
        this->packing->acceptMove();
        moveCounter.increment(true);
        return true;
    } else {
        moveCounter.increment(false);
        return false;
    }
}

bool Simulation::tryScaling(const Interaction &interaction) {
    auto &mt = this->mts.front();

    TriclinicBox oldBox = this->packing->getBox();
    TriclinicBox newBox = this->boxScaler->updateBox(oldBox, mt);
    Expects(newBox.getVolume() != 0);
    double oldV = std::abs(oldBox.getVolume());
    double newV = std::abs(newBox.getVolume());
    double deltaV = newV - oldV;
    double factor = newV/oldV;

    auto N = static_cast<double>(this->packing->size());
    if (interaction.hasSoftPart() || this->areOverlapsCounted) {
        // Soft interaction present - we have a nontrivial energy change, and we always need to try scaling.
        // Same if only hard part, but overlaps are counted, so non-negative energy changes are not guaranteed.
        double dE = this->packing->tryScaling(newBox, interaction);
        double exponent = N * log(factor) - dE / this->temperature - this->pressure * deltaV / this->temperature;
        if (this->unitIntervalDistribution(mt) <= std::exp(exponent)) {
            return true;
        } else {
            this->packing->revertScaling();
            return false;
        }
    } else {
        // No soft interaction - we do not need to try scaling, if Metropolis criterion will kill it anyway
        double exponent = N * log(factor) - this->pressure * deltaV / this->temperature;
        if (this->unitIntervalDistribution(mt) > std::exp(exponent)) {
            return false;
        } else if (this->packing->tryScaling(newBox, interaction) > 0) {
            this->packing->revertScaling();
            return false;
        } else {
            return true;
        }
    }
}

void Simulation::evaluateCounters(Logger &logger) {
    this->evaluateMoleculeMoveCounter(logger);

    if (this->scalingCounter.getMovesSinceEvaluation() >= 100) {
        double rate = this->scalingCounter.getCurrentRate();
        this->scalingCounter.resetCurrent();
        if (rate > 0.2) {
            double prevStepSize = this->boxScaler->getStepSize();
            this->boxScaler->increaseStepSize();
            double newStepSize = this->boxScaler->getStepSize();
            logger.info() << "-- Scaling rate: " << rate << ", step size increased: " << prevStepSize;
            logger << " -> " << newStepSize << std::endl;
        } else if (rate < 0.1) {
            double prevStepSize = this->boxScaler->getStepSize();
            this->boxScaler->decreaseStepSize();
            double newStepSize = this->boxScaler->getStepSize();
            logger.info() << "-- Scaling rate: " << rate << ", step size decreased: " << prevStepSize;
            logger << " -> " << newStepSize << std::endl;
        }
    }
}

void Simulation::evaluateMoleculeMoveCounter(Logger &logger) {
    for (std::size_t i{}; i < this->moveSamplers.size(); i++) {
        auto &moveSampler = *this->moveSamplers[i];
        auto &moveCounter = this->moveCounters[i];
        std::vector<bool>::reference cancelReported = this->adjustmentCancelReported[i];
        std::size_t requestedMoves = moveSampler.getNumOfRequestedMoves(this->packing->size());
        auto moveName = moveSampler.getName();
        moveName.front() = static_cast<char>(toupper(moveName.front()));

        if (moveCounter.getMovesSinceEvaluation() < 100 * requestedMoves)
            continue;

        double rate = moveCounter.getCurrentRate();
        moveCounter.resetCurrent();
        auto oldStepSizes = moveSampler.getStepSizes();
        if (rate > 0.2) {
            if (moveSampler.increaseStepSize()) {
                logger.info() << "-- " << moveName << " rate: " << rate << "; step sizes increased: ";
                auto newStepSizes = moveSampler.getStepSizes();
                printStepSizesChange(logger, oldStepSizes, newStepSizes);
                logger << std::endl;
                cancelReported = false;
            } else {
                if (!cancelReported) {
                    logger.info() << "-- " << moveName << " rate: " << rate << "; increase of step sizes aborted ";
                    logger << "(further notices not displayed)" << std::endl;
                }
                cancelReported = true;
            }
        } else if (rate < 0.1) {
            if (moveSampler.decreaseStepSize()) {
                logger.info() << "-- " << moveName << " rate: " << rate << "; step sizes decreased: ";
                auto newStepSizes = moveSampler.getStepSizes();
                printStepSizesChange(logger, oldStepSizes, newStepSizes);
                logger << std::endl;
                cancelReported = false;
            } else {
                if (!cancelReported) {
                    logger.info() << "-- " << moveName << " rate: " << rate << "; decrease of step sizes aborted ";
                    logger << "(further notices not displayed)" << std::endl;
                }
                cancelReported = true;
            }
        }
    }
}

void Simulation::Counter::increment(bool accepted) {
    this->moves++;
    this->movesSinceEvaluation++;
    if (accepted) {
        this->acceptedMoves++;
        this->acceptedMovesSinceEvaluation++;
    }
}

void Simulation::Counter::reset() {
    this->acceptedMoves = 0;
    this->moves = 0;
    this->acceptedMovesSinceEvaluation = 0;
    this->movesSinceEvaluation = 0;
}

void Simulation::Counter::resetCurrent() {
    this->acceptedMovesSinceEvaluation = 0;
    this->movesSinceEvaluation = 0;
}

double Simulation::Counter::getCurrentRate() const {
    return static_cast<double>(this->acceptedMovesSinceEvaluation) / static_cast<double>(this->movesSinceEvaluation);
}

double Simulation::Counter::getRate() const {
    return static_cast<double>(this->acceptedMoves) / static_cast<double>(this->moves);
}

std::size_t Simulation::Counter::getMovesSinceEvaluation() const {
    return this->movesSinceEvaluation;
}

Simulation::Counter &Simulation::Counter::operator+=(const Simulation::Counter &other) {
    this->acceptedMoves += other.acceptedMoves;
    this->moves += other.moves;
    this->acceptedMovesSinceEvaluation += other.acceptedMovesSinceEvaluation;
    this->movesSinceEvaluation += other.getMovesSinceEvaluation();
    return *this;
}

std::size_t Simulation::Counter::getMoves() const {
    return this->moves;
}

std::size_t Simulation::Counter::getAcceptedMoves() const {
    return this->acceptedMoves;
}

void Simulation::printInlineInfo(std::size_t cycleNumber, const ShapeTraits &traits, Logger &logger,
                                 bool displayOverlaps)
{
    logger.info() << "Performed " << cycleNumber << " cycles; ";
    if (displayOverlaps)
        logger << "overlaps: " << this->packing->getCachedNumberOfOverlaps() << "; ";

    logger << this->observablesCollector->generateInlineObservablesString(*this->packing, traits);
    logger << std::endl;
    logger.verbose() << "Memory usage (bytes): shape: " << this->packing->getShapesMemoryUsage() << ", ";
    logger << "ng: " << this->packing->getNeighbourGridMemoryUsage() << ", ";
    logger << "obs: " << this->observablesCollector->getMemoryUsage() << std::endl;
}

bool Simulation::wasInterrupted() const {
    return sigint_received;
}

void Simulation::accumulateCounters(std::vector<Counter> &out, const std::vector<Counter> &in) {
    for (std::size_t i{}; i < out.size(); i++)
        out[i] += in[i];
}

std::vector<std::size_t> Simulation::calculateMoveTypeAccumulations(std::size_t numParticles) const {
    std::vector<std::size_t> moveTypeAccumulations;
    moveTypeAccumulations.reserve(this->moveSamplers.size());
    std::size_t moveTypeAccumulation = 0;
    for (const auto &moveSampler : this->moveSamplers) {
        moveTypeAccumulation += moveSampler->getNumOfRequestedMoves(numParticles);
        moveTypeAccumulations.push_back(moveTypeAccumulation);
    }

    return moveTypeAccumulations;
}

void Simulation::printStepSizesChange(Logger &logger, const std::vector<std::pair<std::string, double>> &oldStepSizes,
                                      const std::vector<std::pair<std::string, double>> &newStepSizes)
{
    Expects(oldStepSizes.size() == newStepSizes.size());

    for (std::size_t i{}; i < oldStepSizes.size(); i++) {
        auto [oldName, oldStep] = oldStepSizes[i];
        auto [newName, newStep] = newStepSizes[i];
        Expects(oldName == newName);

        logger << oldName << ": " << oldStep << " -> " << newStep;
        if (i < oldStepSizes.size() - 1)
            logger << ", ";
    }
}

std::vector<std::unique_ptr<MoveSampler>> Simulation::makeRototranslation(double translationStepSize,
                                                                          double rotationStepSize)
{
    std::vector<std::unique_ptr<MoveSampler>> result;
    result.push_back(std::make_unique<RototranslationSampler>(translationStepSize, rotationStepSize));
    return result;
}

Simulation::MoveStatistics Simulation::getScalingStatistics() const {
    std::size_t total = this->scalingCounter.getMoves();
    std::size_t accepted = this->scalingCounter.getAcceptedMoves();
    double stepSize = this->boxScaler->getStepSize();

    return MoveStatistics("scaling", total, accepted, {StepSizeData("scaling", stepSize)});
}

std::vector<Simulation::MoveStatistics> Simulation::getMovesStatistics() const {
    std::vector<MoveStatistics> moveGroupsStatistics;

    Assert(this->moveSamplers.size() == this->moveCounters.size());
    for (std::size_t i{}; i < this->moveSamplers.size(); i++) {
        const auto &moveSampler = this->moveSamplers[i];
        const auto &moveCounter  = this->moveCounters[i];

        std::string groupName = moveSampler->getName();
        std::size_t total = moveCounter.getMoves();
        std::size_t accepted = moveCounter.getAcceptedMoves();
        std::vector<StepSizeData> stepSizeData;
        for (auto [moveName, stepSize] : moveSampler->getStepSizes())
            stepSizeData.emplace_back(moveName, stepSize);

        moveGroupsStatistics.emplace_back(groupName, total, accepted, stepSizeData);
    }

    moveGroupsStatistics.push_back(this->getScalingStatistics());

    return moveGroupsStatistics;
}

void Simulation::fixRotationMatrices(const Interaction &interaction, Logger &logger) {
    double maxDeviation = 0;
    std::size_t numFixes = 0;

    std::vector<Shape> shapes(std::cbegin(*this->packing), std::cend(*this->packing));
    TriclinicBox box = this->packing->getBox();

    for (auto &shape : shapes) {
        auto rotation = shape.getOrientation();
        double deviation = Simulation::getRotationMatrixDeviation(rotation);
        if (deviation > maxDeviation)
            maxDeviation = deviation;

        if (deviation > std::pow(1e-14, 2)) {
            Simulation::fixRotationMatrix(rotation);
            shape.setOrientation(rotation);
            numFixes++;
        }
    }

    if (numFixes > 0)
        this->packing->reset(shapes, box, interaction);

    if (!this->areOverlapsCounted && this->packing->countTotalOverlaps(interaction) > 0) {
        logger.error() << "During orientation normalization some overlaps were introduced. Interrupting.";
        logger << std::endl;
        sigint_received = true;
    }

    logger.verbose() << "Orientation normalization was performed. Fixed molecules: " << numFixes << "/";
    logger << this->packing->size() << "; highest deviation: " << maxDeviation << std::endl;
}

void Simulation::fixRotationMatrix(Matrix<3, 3> &rotation) {
    // Iterative algorithm from https://math.stackexchange.com/questions/3292034/normalizing-a-rotation-matrix
    // At most 3 iterations, however usually 1 is enough
    for (std::size_t i{}; i < 3; i++) {
        rotation = 1.5 * rotation - 0.5 * rotation * rotation.transpose() * rotation;
        if (Simulation::getRotationMatrixDeviation(rotation) < std::pow(1e-15, 2))
            break;
    }
}

double Simulation::getRotationMatrixDeviation(const Matrix<3, 3> &rotation) {
    return (rotation * rotation.transpose() - Matrix<3, 3>::identity()).norm2();
}
