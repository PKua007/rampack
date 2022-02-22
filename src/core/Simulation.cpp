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

Simulation::Simulation(std::unique_ptr<Packing> packing, double translationStep, double rotationStep,
                       double scalingStep, unsigned long seed, std::unique_ptr<TriclinicBoxScaler> boxScaler,
                       const std::array<std::size_t, 3> &domainDivisions, bool handleSignals)
        : translationStep{translationStep}, rotationStep{rotationStep}, scalingStep{scalingStep},
          boxScaler{std::move(boxScaler)}, packing{std::move(packing)}, allParticleIndices(this->packing->size()),
          domainDivisions{domainDivisions}
{
    Expects(!this->packing->empty());
    Expects(translationStep > 0);
    Expects(rotationStep > 0);
    Expects(scalingStep > 0);

    this->numDomains = std::accumulate(domainDivisions.begin(), domainDivisions.end(), 1, std::multiplies<>{});
    Expects(this->numDomains > 0);
    Expects(this->numDomains <= this->packing->getMoveThreads());

    this->mts.reserve(this->numDomains);
    for (std::size_t i{}; i < this->numDomains; i++)
        this->mts.emplace_back(seed + i);

    std::iota(this->allParticleIndices.begin(), this->allParticleIndices.end(), 0);

    if (handleSignals) {
        std::signal(SIGINT, sigint_handler);
        std::signal(SIGTERM, sigint_handler);
    }
}

void Simulation::perform(double temperature_, double pressure_, std::size_t thermalisationCycles_,
                         std::size_t averagingCycles_, std::size_t averagingEvery_, std::size_t snapshotEvery_,
                         const ShapeTraits &shapeTraits, std::unique_ptr<ObservablesCollector> observablesCollector_,
                         Logger &logger, std::size_t cycleOffset)
{
    Expects(temperature_ > 0);
    Expects(pressure_ > 0);
    Expects(thermalisationCycles_ > 0);
    Expects(averagingCycles_ > 0);
    Expects(averagingEvery_ > 0 && averagingEvery_ < averagingCycles_);

    this->temperature = temperature_;
    this->pressure = pressure_;
    this->thermalisationCycles = thermalisationCycles_;
    this->averagingCycles = averagingCycles_;
    this->averagingEvery = averagingEvery_;
    this->snapshotEvery = snapshotEvery_;
    this->observablesCollector = std::move(observablesCollector_);
    this->observablesCollector->setThermodynamicParameters(this->temperature, this->pressure);
    this->reset();

    this->totalCycles = cycleOffset;

    const Interaction &interaction = shapeTraits.getInteraction();
    LoggerAdditionalTextAppender loggerAdditionalTextAppender(logger);

    this->shouldAdjustStepSize = true;
    loggerAdditionalTextAppender.setAdditionalText("th");
    logger.info() << "Starting thermalisation..." << std::endl;
    for (std::size_t i{}; i < this->thermalisationCycles; i++) {
        this->performCycle(logger, interaction);
        if (this->totalCycles % this->snapshotEvery == 0)
            this->observablesCollector->addSnapshot(*this->packing, this->totalCycles, shapeTraits);
        if (this->totalCycles % 100 == 0)
            this->printInlineInfo(this->totalCycles, shapeTraits, logger);
        if (sigint_received) {
            logger.warn() << "SIGINT/SIGKILL received, stopping on " << this->totalCycles << " cycle." << std::endl;
            return;
        }
    }

    this->shouldAdjustStepSize = false;
    loggerAdditionalTextAppender.setAdditionalText("av");
    logger.info() << "Starting averaging..." << std::endl;
    for(std::size_t i{}; i < this->averagingCycles; i++) {
        this->performCycle(logger, interaction);
        if (this->totalCycles % this->snapshotEvery == 0)
            this->observablesCollector->addSnapshot(*this->packing, this->totalCycles, shapeTraits);
        if (this->totalCycles % this->averagingEvery == 0)
            this->observablesCollector->addAveragingValues(*this->packing, shapeTraits);
        if (this->totalCycles % 100 == 0)
            this->printInlineInfo(this->totalCycles, shapeTraits, logger);
        if (sigint_received) {
            logger.warn() << "SIGINT/SIGKILL received, stopping on " << this->totalCycles << " cycle." << std::endl;
            return;
        }
    }
}

void Simulation::reset() {
    std::uniform_int_distribution<int>(0, this->packing->size() - 1);
    this->moveCounter.reset();
    this->scalingCounter.reset();
    this->packing->resetCounters();
    this->moveMicroseconds = 0;
    this->scalingMicroseconds = 0;
    this->domainDecompositionMicroseconds = 0;
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

    start = high_resolution_clock::now();
    bool wasScaled = this->tryScaling(interaction);
    this->scalingCounter.increment(wasScaled);
    end = high_resolution_clock::now();
    this->scalingMicroseconds += duration<double, std::micro>(end - start).count();

    if (this->shouldAdjustStepSize)
        this->evaluateCounters(logger);

    this->performedCycles++;
    this->totalCycles++;
}

void Simulation::performMovesWithoutDomainDivision(const Interaction &interaction) {
    for (std::size_t i{}; i < this->packing->size(); i++) {
        bool wasMoved = tryMove(interaction, this->allParticleIndices);
        this->moveCounter.increment(wasMoved);
    }
}

void Simulation::performMovesWithDomainDivision(const Interaction &interaction) {
    const auto &packingBox = this->packing->getBox();
    auto &mt = this->mts[_OMP_THREAD_ID];

    Vector<3> randomOrigin{this->unitIntervalDistribution(mt),
                           this->unitIntervalDistribution(mt),
                           this->unitIntervalDistribution(mt)};
    randomOrigin = packingBox.relativeToAbsolute(randomOrigin);
    const auto &neighbourGridCellDivisions = this->packing->getNeighbourGridCellDivisions();
    Counter tempMoveCounter;

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    DomainDecomposition domainDecomposition(*this->packing, interaction, this->domainDivisions,
                                            neighbourGridCellDivisions, randomOrigin);
    auto end = high_resolution_clock::now();
    this->domainDecompositionMicroseconds += duration<double, std::micro>(end - start).count();

    #pragma omp declare reduction (+ : Counter : omp_out += omp_in)
    #pragma omp parallel for shared(domainDecomposition, interaction) default(none) collapse(3) \
            reduction(+ : tempMoveCounter) num_threads(this->packing->getMoveThreads())
    for (std::size_t i = 0; i < this->domainDivisions[0]; i++) {
        for (std::size_t j = 0; j < this->domainDivisions[1]; j++) {
            for (std::size_t k = 0; k < this->domainDivisions[2]; k++) {
                std::array<std::size_t, 3> coords = {i, j, k};

                const auto &domainParticleIndices = domainDecomposition.getParticlesInRegion(coords);
                auto activeDomain = domainDecomposition.getActiveDomainBounds(coords);
                if (domainParticleIndices.empty())
                    continue;

                std::size_t numMoves = this->packing->size() / this->numDomains;
                for (std::size_t x{}; x < numMoves; x++) {
                    bool wasMoved = tryMove(interaction, domainParticleIndices, activeDomain);
                    tempMoveCounter.increment(wasMoved);
                }
            }
        }
    }

    this->moveCounter += tempMoveCounter;
}

bool Simulation::tryTranslation(const Interaction &interaction, const std::vector<std::size_t> &particleIndices,
                                std::optional<ActiveDomain> boundaries)
{
    auto &mt = this->mts[_OMP_THREAD_ID];

    Vector<3> translation{2*this->unitIntervalDistribution(mt) - 1,
                          2*this->unitIntervalDistribution(mt) - 1,
                          2*this->unitIntervalDistribution(mt) - 1};
    translation *= this->translationStep;

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIndices.size() - 1);
    double dE = this->packing->tryTranslation(particleIndices[particleDistribution(mt)], translation, interaction,
                                              boundaries);
    if (this->unitIntervalDistribution(mt) <= std::exp(-dE / this->temperature)) {
        this->packing->acceptTranslation();
        return true;
    } else {
        return false;
    }
}

bool Simulation::tryRotation(const Interaction &interaction, const std::vector<std::size_t> &particleIndices) {
    auto &mt = this->mts[_OMP_THREAD_ID];

    Vector<3> axis;
    do {
        axis[0] = 2*this->unitIntervalDistribution(mt) - 1;
        axis[1] = 2*this->unitIntervalDistribution(mt) - 1;
        axis[2] = 2*this->unitIntervalDistribution(mt) - 1;
    } while (axis.norm2() > 1);
    double angle = (2*this->unitIntervalDistribution(mt) - 1) * this->rotationStep;
    auto rotation = Matrix<3, 3>::rotation(axis.normalized(), angle);

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIndices.size() - 1);
    double dE = this->packing->tryRotation(particleIndices[particleDistribution(mt)], rotation, interaction);
    if (this->unitIntervalDistribution(mt) <= std::exp(-dE / this->temperature)) {
        this->packing->acceptRotation();
        return true;
    } else {
        return false;
    }
}

bool Simulation::tryMove(const Interaction &interaction, const std::vector<std::size_t> &particleIndices,
                         std::optional<ActiveDomain> boundaries)
{
    auto &mt = this->mts[_OMP_THREAD_ID];

    Vector<3> translation{2*this->unitIntervalDistribution(mt) - 1,
                          2*this->unitIntervalDistribution(mt) - 1,
                          2*this->unitIntervalDistribution(mt) - 1};
    translation *= this->translationStep;

    Vector<3> axis;
    do {
        axis[0] = 2*this->unitIntervalDistribution(mt) - 1;
        axis[1] = 2*this->unitIntervalDistribution(mt) - 1;
        axis[2] = 2*this->unitIntervalDistribution(mt) - 1;
    } while (axis.norm2() > 1);
    double angle = (2*this->unitIntervalDistribution(mt) - 1) * std::min(this->rotationStep, M_PI);
    auto rotation = Matrix<3, 3>::rotation(axis.normalized(), angle);

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIndices.size() - 1);
    double dE = this->packing->tryMove(particleIndices[particleDistribution(mt)], translation, rotation, interaction,
                                       boundaries);
    if (this->unitIntervalDistribution(mt) <= std::exp(-dE / this->temperature)) {
        this->packing->acceptMove();
        return true;
    } else {
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
    if (interaction.hasSoftPart()) {
        // Soft interaction present - we have a nontrivial energy change an we always need to try scaling
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
        } else if (this->packing->tryScaling(newBox, interaction) != 0) {
            this->packing->revertScaling();
            return false;
        } else {
            return true;
        }
    }
}

void Simulation::evaluateCounters(Logger &logger) {
    if (this->moveCounter.getMovesSinceEvaluation() >= 100 * this->packing->size()) {
        double rate = this->moveCounter.getCurrentRate();
        this->moveCounter.resetCurrent();
        if (rate > 0.2) {
            const auto &dimensions = this->packing->getBox().getHeights();
            double minDimension = *std::min_element(dimensions.begin(), dimensions.end());
            // Current policy: adjust translations and rotations at the same time - the ratio from the config file
            // is kept. Translation step can be as large as the packing, but not larger. Rotation step would usually
            // be > M_PI then anyway
            if (this->translationStep * 1.1 <= minDimension) {
                this->translationStep *= 1.1;
                this->rotationStep *= 1.1;
                logger.info() << "Translation rate: " << rate << ", adjusting: "  << (this->translationStep / 1.1);
                logger << " -> " << this->translationStep << std::endl;
                logger.info() << "Rotation rate: " << rate << ", adjusting: "  << (this->rotationStep / 1.1);
                logger << " -> " << this->rotationStep << std::endl;
            }
        } else if (rate < 0.1) {
            this->translationStep /= 1.1;
            this->rotationStep /= 1.1;
            logger.info() << "Translation rate: " << rate << ", adjusting: " << (this->translationStep * 1.1);
            logger << " -> " << this->translationStep << std::endl;
            logger.info() << "Rotation rate: " << rate << ", adjusting: " << (this->rotationStep * 1.1);
            logger << " -> " << this->rotationStep << std::endl;
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

void Simulation::printInlineInfo(std::size_t cycleNumber, const ShapeTraits &traits, Logger &logger) {
    logger.info() << "Performed " << cycleNumber << " cycles; ";
    logger << this->observablesCollector->generateInlineObservablesString(*this->packing, traits);
    logger << std::endl;
    logger.verbose() << "Memory usage (bytes): shape: " << this->packing->getShapesMemoryUsage() << ", ";
    logger << "ng: " << this->packing->getNeighbourGridMemoryUsage() << ", ";
    logger << "obs: " << this->observablesCollector->getMemoryUsage() << std::endl;
}

bool Simulation::wasInterrupted() const {
    return sigint_received;
}
