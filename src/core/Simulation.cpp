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

Simulation::Simulation(std::unique_ptr<Packing> packing, std::vector<std::unique_ptr<MoveSampler>> moveSamplers,
                       double scalingStep, unsigned long seed, std::unique_ptr<TriclinicBoxScaler> boxScaler,
                       const std::array<std::size_t, 3> &domainDivisions, bool handleSignals)
        : moveSamplers{std::move(moveSamplers)}, scalingStep{scalingStep}, boxScaler{std::move(boxScaler)},
          packing{std::move(packing)}, allParticleIndices(this->packing->size()), domainDivisions{domainDivisions}
{
    Expects(!this->packing->empty());
    Expects(scalingStep > 0);
    Expects(!this->moveSamplers.empty());

    this->numDomains = std::accumulate(domainDivisions.begin(), domainDivisions.end(), 1, std::multiplies<>{});
    Expects(this->numDomains > 0);
    Expects(this->numDomains <= this->packing->getMoveThreads());

    this->mts.reserve(this->numDomains);
    for (std::size_t i{}; i < this->numDomains; i++)
        this->mts.emplace_back(seed + i);

    this->moveCounters.resize(this->moveSamplers.size());

    std::iota(this->allParticleIndices.begin(), this->allParticleIndices.end(), 0);

    if (handleSignals) {
        std::signal(SIGINT, sigint_handler);
        std::signal(SIGTERM, sigint_handler);
    }
}

Simulation::Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep,
                       double scalingStep, unsigned long seed, std::unique_ptr<TriclinicBoxScaler> boxScaler,
                       const std::array<std::size_t, 3> &domainDivisions, bool handleSignals)
        : Simulation(std::move(packing), Simulation::makeRototranslation(translationStep, rotationStep),
                     scalingStep, seed, std::move(boxScaler), domainDivisions, handleSignals)
{ }

void Simulation::integrate(double temperature_, double pressure_, std::size_t thermalisationCycles,
                           std::size_t averagingCycles, std::size_t averagingEvery, std::size_t snapshotEvery,
                           const ShapeTraits &shapeTraits, std::unique_ptr<ObservablesCollector> observablesCollector_,
                           Logger &logger, std::size_t cycleOffset)
{
    Expects(temperature_ > 0);
    Expects(pressure_ > 0);
    if (averagingCycles > 0)
        Expects(averagingEvery > 0 && averagingEvery < averagingCycles);

    this->temperature = temperature_;
    this->pressure = pressure_;
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

    this->shouldAdjustStepSize = true;
    loggerAdditionalTextAppender.setAdditionalText("th");
    if (thermalisationCycles == 0) {
        logger.info() << "Thermalization skipped." << std::endl;
    } else {
        logger.info() << "Starting thermalisation..." << std::endl;
        for (std::size_t i{}; i < thermalisationCycles; i++) {
            this->performCycle(logger, interaction);
            if (this->totalCycles % snapshotEvery == 0)
                this->observablesCollector->addSnapshot(*this->packing, this->totalCycles, shapeTraits);
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
            this->performCycle(logger, interaction);
            if (this->totalCycles % snapshotEvery == 0)
                this->observablesCollector->addSnapshot(*this->packing, this->totalCycles, shapeTraits);
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

void Simulation::relaxOverlaps(double temperature_, double pressure_, std::size_t snapshotEvery,
                               const ShapeTraits &shapeTraits,
                               std::unique_ptr<ObservablesCollector> observablesCollector_, Logger &logger,
                               std::size_t cycleOffset)
{
    Expects(temperature_ > 0);
    Expects(pressure_ > 0);

    this->temperature = temperature_;
    this->pressure = pressure_;
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
        this->performCycle(logger, interaction);
        if (this->totalCycles % snapshotEvery == 0)
            this->observablesCollector->addSnapshot(*this->packing, this->totalCycles, shapeTraits);
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

void Simulation::performCycle(Logger &logger, const Interaction &interaction) {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    if (this->numDomains == 1)
        this->performMovesWithoutDomainDivision(interaction);
    else
        this->performMovesWithDomainDivision(interaction);
    auto end = high_resolution_clock::now();
    this->moveMicroseconds += duration<double, std::micro>(end - start).count();

    #ifdef SIMULATION_SANITIZE_CACHED_OVERLAPS
        if (this->areOverlapsCounted)
            Assert(this->packing->getCachedNumberOfOverlaps() == this->packing->countTotalOverlaps(interaction, false));
    #endif

    start = high_resolution_clock::now();
    bool wasScaled = this->tryScaling(interaction);
    this->scalingCounter.increment(wasScaled);
    end = high_resolution_clock::now();
    this->scalingMicroseconds += duration<double, std::micro>(end - start).count();

    #ifdef SIMULATION_SANITIZE_CACHED_OVERLAPS
        if (this->areOverlapsCounted)
            Assert(this->packing->getCachedNumberOfOverlaps() == this->packing->countTotalOverlaps(interaction, false));
    #endif

    if (this->shouldAdjustStepSize)
        this->evaluateCounters(logger);

    this->performedCycles++;
    this->totalCycles++;
}

void Simulation::performMovesWithoutDomainDivision(const Interaction &interaction) {
    auto moveTypeAccumulations = this->calculateMoveTypeAccumulations(this->packing->size());
    std::size_t numMoves = moveTypeAccumulations.back();
    for (std::size_t i{}; i < numMoves; i++)
        this->tryMove(interaction, this->allParticleIndices, this->moveCounters, moveTypeAccumulations);
}

void Simulation::performMovesWithDomainDivision(const Interaction &interaction) {
    const auto &packingBox = this->packing->getBox();
    auto &mt = this->mts[_OMP_THREAD_ID];

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
    #pragma omp parallel for shared(domainDecomposition, interaction) default(none) collapse(3) \
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
                    this->tryMove(interaction, domainParticleIndices, tempMoveCounters, moveTypeAccumulations,
                                  activeDomain);
                }
            }
        }
    }

    this->packing->resetNGRaceConditionSanitizer();

    Simulation::accumulateCounters(this->moveCounters, tempMoveCounters);
}

bool Simulation::tryMove(const Interaction &interaction, const std::vector<std::size_t> &particleIndices,
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
    auto move = moveSampler->sampleMove(particleIndices, mt);
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

    auto &moveCounter = this->moveCounters[moveType];
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
    TriclinicBox newBox = this->boxScaler->updateBox(oldBox, this->scalingStep, mt);
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
    for (std::size_t i{}; i < this->moveSamplers.size(); i++) {
        auto &moveSampler = *this->moveSamplers[i];
        auto &moveCounter = this->moveCounters[i];
        std::size_t requestedMoves = moveSampler.getNumOfRequestedMoves(this->packing->size());

        if (moveCounter.getMovesSinceEvaluation() >= 100 * requestedMoves) {
            double rate = moveCounter.getCurrentRate();
            moveCounter.resetCurrent();
            auto oldStepSizes = moveSampler.getStepSizes();
            if (rate > 0.2) {
                if (moveSampler.increaseStepSize()) {
                    logger.info() << "Molecule move step sizes increased: ";
                    auto newStepSizes = moveSampler.getStepSizes();
                    Simulation::printStepSizesChange(logger, oldStepSizes, newStepSizes);
                    logger << std::endl;
                } else {
                    logger.info() << "The increase of molecule move step sizes aborted" << std::endl;
                }
            } else if (rate < 0.1) {
                if (moveSampler.decreaseStepSize()) {
                    logger.info() << "Molecule move step sizes decreased: ";
                    auto newStepSizes = moveSampler.getStepSizes();
                    Simulation::printStepSizesChange(logger, oldStepSizes, newStepSizes);
                    logger << std::endl;
                } else {
                    logger.info() << "The decrease of molecule move step sizes aborted" << std::endl;
                }
            }
        }
    }

    if (this->scalingCounter.getMovesSinceEvaluation() >= 100) {
        double rate = this->scalingCounter.getCurrentRate();
        this->scalingCounter.resetCurrent();
        if (rate > 0.2) {
            this->scalingStep *= 1.1;
            logger.info() << "Scaling rate: " << rate << ", adjusting: " << (this->scalingStep / 1.1);
            logger << " -> " << this->scalingStep << std::endl;
        } else if (rate < 0.1) {
            this->scalingStep /= 1.1;
            logger.info() << "Scaling rate: " << rate << ", adjusting: " << (this->scalingStep * 1.1);
            logger << " -> " << this->scalingStep << std::endl;
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

double Simulation::getMoveAcceptanceRate() const {
    double rate = std::accumulate(this->moveCounters.begin(), this->moveCounters.end(), 0.0,
                                  [](double rate, const Counter &counter) { return rate + counter.getRate(); });
    return rate / this->moveCounters.size();
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
