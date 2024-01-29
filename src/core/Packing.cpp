//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <ostream>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <functional>

#include "Packing.h"
#include "utils/Exceptions.h"
#include "utils/Utils.h"
#include "utils/ParseUtils.h"
#include "core/io/RamsnapReader.h"
#include "core/io/RamsnapWriter.h"


namespace {
    // Helper class for precomputed boundary conditions translations
    class HardcodedTranslation : public BoundaryConditions {
    private:
        Vector<3> translation;

    public:
        explicit HardcodedTranslation(const Vector<3> &translation) : translation{translation} { }

        void setBox(const TriclinicBox &) override { }

        [[nodiscard]] Vector<3> getCorrection(const Vector<3> &) const override {
            throw std::runtime_error("HardcodedTranslation::getCorrection");
        }

        [[nodiscard]] Vector<3> getTranslation(const Vector<3> &, const Vector<3> &) const override {
            return this->translation;
        }

        [[nodiscard]] double getDistance2(const Vector<3> &position1, const Vector<3> &position2) const override {
            return (position2 + this->translation - position1).norm2();
        }
    };
}

Packing::Packing(const TriclinicBox &box, std::vector<Shape> shapes, std::unique_ptr<BoundaryConditions> bc,
                 const Interaction &interaction, std::size_t moveThreads, std::size_t scalingThreads)
        : bc{std::move(bc)}
{
    this->moveThreads = (moveThreads == 0 ? OMP_MAXTHREADS : moveThreads);
    this->scalingThreads = (scalingThreads == 0 ? OMP_MAXTHREADS : scalingThreads);

    this->reset(std::move(shapes), box, interaction);
}

Packing::Packing(std::unique_ptr<BoundaryConditions> bc, std::size_t moveThreads, std::size_t scalingThreads)
        : bc{std::move(bc)}
{
    this->moveThreads = (moveThreads == 0 ? OMP_MAXTHREADS : moveThreads);
    this->scalingThreads = (scalingThreads == 0 ? OMP_MAXTHREADS : scalingThreads);

    // temp positions at the back so that Packing::end() works
    this->shapePositions.resize(this->moveThreads);

    this->lastAlteredParticleIdx.resize(this->moveThreads, 0);
    this->lastMoveOverlapDeltas.resize(this->moveThreads, 0);
}

void Packing::reset(std::vector<Shape> newShapes, const TriclinicBox &newBox, const Interaction &newInteraction) {
    Expects(newBox.getVolume() != 0);
    Expects(newInteraction.getRangeRadius() > 0);
    Expects(!newShapes.empty());
    Expects(Packing::areShapesWithinBox(newShapes, newBox));

    this->box = newBox;
    this->interactionRange = newInteraction.getRangeRadius();
    this->shapeDataSize = newInteraction.getShapeDataSize();

    std::vector<Vector<3>> newPositions(newShapes.size() + this->moveThreads);
    std::vector<Matrix<3, 3>> newOrientations(newShapes.size() + this->moveThreads);
    std::vector<std::byte> newDatas((newShapes.size() + this->moveThreads) * this->shapeDataSize);

    for (std::size_t i{}; i < newShapes.size(); i++) {
        const auto &shape = newShapes[i];
        newPositions[i] = shape.getPosition();
        newOrientations[i] = shape.getOrientation();

        const auto &shapeData = shape.getData();
        Assert(shapeData.getSize() == this->shapeDataSize);
        const auto *data = shapeData.raw();
        std::copy(data, data + this->shapeDataSize,
                  newDatas.begin() + static_cast<std::ptrdiff_t>(i * this->shapeDataSize));
    }

    std::swap(this->shapePositions, newPositions);
    std::swap(this->shapeOrientations, newOrientations);
    std::swap(this->shapeDatas, newDatas);

    this->lastAlteredParticleIdx.resize(this->moveThreads, 0);
    this->lastMoveOverlapDeltas.resize(this->moveThreads, 0);
    this->bc->setBox(this->box);
    this->setupForInteraction(newInteraction);
}

double Packing::tryTranslation(std::size_t particleIdx, Vector<3> translation, const Interaction &interaction,
                               std::optional<ActiveDomain> boundaries)
{
    Expects(particleIdx < this->size());
    Expects(interaction.getRangeRadius() <= this->interactionRange);

    std::size_t tempParticleIdx = this->size() + OMP_THREAD_ID;
    this->lastAlteredParticleIdx[OMP_THREAD_ID] = particleIdx;
    this->copyShape(particleIdx, tempParticleIdx);

    this->translateShapeWithoutInteractionCenters(tempParticleIdx, translation);

    if (boundaries.has_value() && !boundaries->isInside( this->shapePositions[tempParticleIdx]))
        return std::numeric_limits<double>::infinity();

    if (this->numInteractionCentres != 0) {
        this->prepareTempInteractionCentres(particleIdx);
        this->recalculateAbsoluteInteractionCentres(tempParticleIdx);
    }

    double overlapEnergy = this->calculateMoveOverlapEnergy(particleIdx, tempParticleIdx, interaction);
    if (overlapEnergy != 0)
        return overlapEnergy;

    double initialEnergy = this->calculateParticleEnergy(particleIdx, particleIdx, interaction);
    double finalEnergy = this->calculateParticleEnergy(particleIdx, tempParticleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryRotation(std::size_t particleIdx, const Matrix<3, 3> &rotation, const Interaction &interaction) {
    Expects(particleIdx < this->size());
    Expects(interaction.getRangeRadius() <= this->interactionRange);

    std::size_t tempParticleIdx = this->size() + OMP_THREAD_ID;
    this->lastAlteredParticleIdx[OMP_THREAD_ID] = particleIdx;
    this->copyShape(particleIdx, tempParticleIdx);

    this->rotateShapeWithoutInteractionCenters(tempParticleIdx, rotation);
    if (this->numInteractionCentres != 0) {
        this->prepareTempInteractionCentres(particleIdx);
        this->rotateTempInteractionCentres(rotation);
        this->recalculateAbsoluteInteractionCentres(tempParticleIdx);
    }

    double overlapEnergy = this->calculateMoveOverlapEnergy(particleIdx, tempParticleIdx, interaction);
    if (overlapEnergy != 0)
        return overlapEnergy;

    double initialEnergy = this->calculateParticleEnergy(particleIdx, particleIdx, interaction);
    double finalEnergy = this->calculateParticleEnergy(particleIdx, tempParticleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryMove(std::size_t particleIdx, const Vector<3> &translation, const Matrix<3, 3> &rotation,
                        const Interaction &interaction, std::optional<ActiveDomain> boundaries)
{
    Expects(particleIdx < this->size());
    Expects(interaction.getRangeRadius() <= this->interactionRange);

    std::size_t tempParticleIdx = this->size() + OMP_THREAD_ID;
    this->lastAlteredParticleIdx[OMP_THREAD_ID] = particleIdx;
    this->copyShape(particleIdx, tempParticleIdx);

    this->translateShapeWithoutInteractionCenters(tempParticleIdx, translation);

    if (boundaries.has_value() && !boundaries->isInside(this->shapePositions[tempParticleIdx]))
        return std::numeric_limits<double>::infinity();

    this->rotateShapeWithoutInteractionCenters(tempParticleIdx, rotation);
    if (this->numInteractionCentres != 0) {
        this->prepareTempInteractionCentres(particleIdx);
        this->rotateTempInteractionCentres(rotation);
        this->recalculateAbsoluteInteractionCentres(tempParticleIdx);
    }

    double overlapEnergy = this->calculateMoveOverlapEnergy(particleIdx, tempParticleIdx, interaction);
    if (overlapEnergy != 0)
        return overlapEnergy;

    double initialEnergy = this->calculateParticleEnergy(particleIdx, particleIdx, interaction);
    double finalEnergy = this->calculateParticleEnergy(particleIdx, tempParticleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryScaling(const std::array<double, 3> &scaleFactor, const Interaction &interaction) {
    TriclinicBox newBox = this->box;
    newBox.scale(scaleFactor);
    return this->tryScaling(newBox, interaction);
}

double Packing::tryScaling(const TriclinicBox &newBox, const Interaction &interaction) {
    Expects(newBox.getVolume() != 0);
    Expects(interaction.getRangeRadius() <= this->interactionRange);
    this->lastBox = this->box;
    this->lastShapePositions = this->shapePositions;

    double initialEnergy = this->getTotalEnergy(interaction);
    this->lastScalingNumOverlaps = this->numOverlaps;

    this->box = newBox;
    this->bc->setBox(this->box);
    for (std::size_t i = 0; i < this->size(); i++) {
        auto &shapePosition = this->shapePositions[i];
        shapePosition = this->box.relativeToAbsolute(this->lastBox.absoluteToRelative(shapePosition));
    }
    std::swap(this->neighbourGrid, this->tempNeighbourGrid);
    if (this->numInteractionCentres != 0)
        this->recalculateAbsoluteInteractionCentres();
    this->rebuildNeighbourGrid();

    static constexpr double INF = std::numeric_limits<double>::infinity();
    if (interaction.hasHardPart()) {
        if (this->overlapCounting) {
            this->numOverlaps = this->countTotalOverlaps(interaction, false);
            int overlapDelta = static_cast<int>(this->numOverlaps) - this->lastScalingNumOverlaps;
            if (overlapDelta < 0)
                return -INF;
            else if (overlapDelta > 0)
                return INF;
        } else {
            bool cannotOverlap = interaction.isConvex() && Packing::isBoxUpscaled(this->lastBox, this->box);
            if (!cannotOverlap && this->countTotalOverlaps(interaction, true) > 0)
                return INF;
        }
    }

    double finalEnergy = this->getTotalEnergy(interaction);
    return finalEnergy - initialEnergy;
}

Shape Packing::operator[](std::size_t i) const {
    Expects(i < this->size());
    return this->generateShapeView(i);
}

Shape Packing::front() const {
    Expects(!this->empty());
    return this->generateShapeView(0);
}

Shape Packing::back() const {
    Expects(!this->empty());
    return this->generateShapeView(this->size() - 1);
}

double Packing::getPackingFraction(double shapeVolume) const {
    Expects(shapeVolume >= 0);
    return this->getNumberDensity() * shapeVolume;
}

double Packing::getNumberDensity() const {
    return this->size() / this->getVolume();
}

void Packing::acceptTranslation() {
    int threadId = OMP_THREAD_ID;

    std::size_t lastAlteredIdx = this->lastAlteredParticleIdx[threadId];
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->remove(lastAlteredIdx, this->shapePositions[lastAlteredIdx]);
        else
            this->removeInteractionCentresFromNeighbourGrid(lastAlteredIdx);
    }

    this->shapePositions[lastAlteredIdx] = this->shapePositions[this->size() + threadId];
    if (this->numInteractionCentres != 0)
        this->acceptTempInteractionCentres();

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->add(lastAlteredIdx, this->shapePositions[lastAlteredIdx]);
        else
            this->addInteractionCentresToNeighbourGrid(lastAlteredIdx);
    }

    if (this->overlapCounting) {
        #pragma omp critical
        this->numOverlaps += this->lastMoveOverlapDeltas[threadId];
    }
}

void Packing::acceptRotation() {
    int threadId = OMP_THREAD_ID;

    std::size_t lastAlteredIdx = this->lastAlteredParticleIdx[threadId];
    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0)
        this->removeInteractionCentresFromNeighbourGrid(lastAlteredIdx);

    this->shapeOrientations[lastAlteredIdx] = this->shapeOrientations[size() + threadId];
    if (this->numInteractionCentres != 0)
        this->acceptTempInteractionCentres();

    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0)
        this->addInteractionCentresToNeighbourGrid(lastAlteredIdx);

    if (this->overlapCounting) {
        #pragma omp critical
        this->numOverlaps += this->lastMoveOverlapDeltas[threadId];
    }
}

void Packing::acceptMove() {
    int threadId = OMP_THREAD_ID;

    std::size_t lastAlteredIdx = this->lastAlteredParticleIdx[threadId];
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->remove(lastAlteredIdx, this->shapePositions[lastAlteredIdx]);
        else
            this->removeInteractionCentresFromNeighbourGrid(lastAlteredIdx);
    }

    std::size_t tempParticleIdx = this->size() + threadId;
    this->shapePositions[lastAlteredIdx] = this->shapePositions[tempParticleIdx];
    this->shapeOrientations[lastAlteredIdx] = this->shapeOrientations[tempParticleIdx];
    if (this->numInteractionCentres != 0)
        this->acceptTempInteractionCentres();

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->add(lastAlteredIdx, this->shapePositions[lastAlteredIdx]);
        else
            this->addInteractionCentresToNeighbourGrid(lastAlteredIdx);
    }

    if (this->overlapCounting) {
        #pragma omp critical
        this->numOverlaps += this->lastMoveOverlapDeltas[threadId];
    }
}

double Packing::calculateMoveOverlapEnergy(size_t particleIdx, size_t tempParticleIdx, const Interaction &interaction) {
    static constexpr double INF = std::numeric_limits<double>::infinity();

    if (interaction.hasHardPart()) {
        if (this->overlapCounting) {
            std::size_t initialOverlaps = this->countParticleOverlaps(particleIdx, particleIdx, interaction, false);
            std::size_t finalOverlaps = this->countParticleOverlaps(particleIdx, tempParticleIdx, interaction, false);
            auto &lastMoveOverlapDelta = this->lastMoveOverlapDeltas[OMP_THREAD_ID];
            lastMoveOverlapDelta = static_cast<int>(finalOverlaps) - initialOverlaps;
            if (lastMoveOverlapDelta < 0)
                return -INF;
            else if (lastMoveOverlapDelta > 0)
                return INF;
        } else {
            if (this->countParticleOverlaps(particleIdx, tempParticleIdx, interaction, true) > 0)
                return INF;
        }
    }
    return 0;
}

void Packing::addInteractionCentresToNeighbourGrid(std::size_t particleIdx) {
    for (size_t i{}; i < this->numInteractionCentres; i++) {
        std::size_t centreIdx = particleIdx * this->numInteractionCentres + i;
        this->neighbourGrid->add(centreIdx, this->absoluteInteractionCentres[centreIdx]);
    }
}

void Packing::removeInteractionCentresFromNeighbourGrid(std::size_t particleIdx) {
    for (size_t i{}; i < this->numInteractionCentres; i++) {
        std::size_t centreIdx = particleIdx * this->numInteractionCentres + i;
        this->neighbourGrid->remove(centreIdx, this->absoluteInteractionCentres[centreIdx]);
    }
}

void Packing::acceptTempInteractionCentres() {
    std::size_t fromOrigin = (this->size() + OMP_THREAD_ID) * this->numInteractionCentres;
    std::size_t toOrigin = this->lastAlteredParticleIdx[OMP_THREAD_ID] * this->numInteractionCentres;
    for (std::size_t i{}; i < this->numInteractionCentres; i++) {
        std::size_t toCentreIdx = toOrigin + i;
        std::size_t fromCentreIdx = fromOrigin + i;
        this->interactionCentres[toCentreIdx] = this->interactionCentres[fromCentreIdx];
        this->absoluteInteractionCentres[toCentreIdx] = this->absoluteInteractionCentres[fromCentreIdx];
    }
}

void Packing::prepareTempInteractionCentres(std::size_t particleIdx) {
    std::size_t fromOrigin = particleIdx * this->numInteractionCentres;
    std::size_t toOrigin = (this->size() + OMP_THREAD_ID) * this->numInteractionCentres;
    for (std::size_t i{}; i < this->numInteractionCentres; i++)
        this->interactionCentres[toOrigin + i] = this->interactionCentres[fromOrigin + i];
}

void Packing::rotateTempInteractionCentres(const Matrix<3, 3> &rotation) {
    std::size_t idxOrigin = (this->size() + OMP_THREAD_ID) * this->numInteractionCentres;
    for (std::size_t i{}; i < this->numInteractionCentres; i++)
        this->interactionCentres[idxOrigin + i] = rotation * this->interactionCentres[idxOrigin + i];
}

void Packing::revertScaling() {
    // We do not need to revert orientations and shape data, because they didn't change
    this->shapePositions = this->lastShapePositions;
    this->box = this->lastBox;
    this->bc->setBox(this->box);
    std::swap(this->neighbourGrid, this->tempNeighbourGrid);
    if (this->numInteractionCentres != 0)
        this->recalculateAbsoluteInteractionCentres();
    this->numOverlaps = this->lastScalingNumOverlaps;
}

std::size_t Packing::countParticleOverlaps(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                           const Interaction &interaction, bool earlyExit) const
{
    std::size_t overlapsCounted{};

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            Vector<3> pos = this->shapePositions[tempParticleIdx];
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos)) {
                HardcodedTranslation cellTranslation(cell.getTranslation());
                for (auto j: cell.getNeighbours()) {
                    if (originalParticleIdx == j)
                        continue;

                    if (interaction.overlapBetween(this->shapePositions[tempParticleIdx],
                                                   this->shapeOrientations[tempParticleIdx],
                                                   this->getShapeDataPtr(tempParticleIdx),
                                                   0,
                                                   this->shapePositions[j],
                                                   this->shapeOrientations[j],
                                                   this->getShapeDataPtr(j),
                                                   0,
                                                   cellTranslation))
                    {
                        if (earlyExit) return 1;
                        overlapsCounted++;
                    }
                }
            }
        } else {
            for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
                std::size_t centreOverlaps = this->countInteractionCentreOverlapsWithNG(originalParticleIdx,
                                                                                        tempParticleIdx, centre1,
                                                                                        interaction, earlyExit);
                if (earlyExit && centreOverlaps > 0)
                    return centreOverlaps;

                overlapsCounted += centreOverlaps;
            }
        }
    } else {
        for (std::size_t j{}; j < this->size(); j++) {
            if (originalParticleIdx == j)
                continue;
            std::size_t particlesOverlaps = this->countOverlapsBetweenParticlesWithoutNG(tempParticleIdx, j,
                                                                                         interaction, earlyExit);
            if (earlyExit && particlesOverlaps)
                return particlesOverlaps;

            overlapsCounted += particlesOverlaps;
        }
    }

    std::size_t wallOverlaps{};
    if (this->hasAnyWalls)
        wallOverlaps = this->countParticleWallOverlaps(tempParticleIdx, interaction, earlyExit);

    return overlapsCounted + wallOverlaps;
}

std::size_t Packing::countTotalOverlapsNGCellHelper(const std::array<std::size_t, 3> &coord,
                                                    const Interaction &interaction, bool earlyExit) const
{
    std::size_t overlapsCounted{};

    if (this->numInteractionCentres == 0) {
        const auto &cellView = this->neighbourGrid->getCell(coord);
        for (auto cellIt1 = cellView.begin(); cellIt1 != cellView.end(); cellIt1++) {
            std::size_t particleIdx1 = *cellIt1;
            const auto &pos1 = this->shapePositions[particleIdx1];
            const auto &orientation1 = this->shapeOrientations[particleIdx1];
            const auto *data1 = this->getShapeDataPtr(particleIdx1);

            // Overlaps within the cell
            HardcodedTranslation noTranslation({});
            for (auto cellIt2 = std::next(cellIt1); cellIt2 != cellView.end(); cellIt2++) {
                std::size_t particleIdx2 = *cellIt2;
                const auto &pos2 = this->shapePositions[particleIdx2];
                const auto &orientation2 = this->shapeOrientations[particleIdx2];
                const auto *data2 = this->getShapeDataPtr(particleIdx2);
                if (interaction.overlapBetween(pos1, orientation1, data1, 0,
                                               pos2, orientation2, data2, 0,
                                               noTranslation))
                {
                    if (earlyExit) return 1;
                    overlapsCounted++;
                }
            }

            // Overlaps with other cells (but only with a half to avoid redundant checks)
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(coord, true)) {
                HardcodedTranslation cellTranslation(cell.getTranslation());
                for (auto particleIdx2 : cell.getNeighbours()) { // NOLINT(readability-use-anyofallof)
                    const auto &pos2 = this->shapePositions[particleIdx2];
                    const auto &orientation2 = this->shapeOrientations[particleIdx2];
                    const auto *data2 = this->getShapeDataPtr(particleIdx2);
                    if (interaction.overlapBetween(pos1, orientation1, data1, 0,
                                                   pos2, orientation2, data2, 0,
                                                   cellTranslation))
                    {
                        if (earlyExit) return 1;
                        overlapsCounted++;
                    }
                }
            }
        }
    } else {
        const auto &cellView = this->neighbourGrid->getCell(coord);
        for (auto cellIt1 = cellView.begin(); cellIt1 != cellView.end(); cellIt1++) {
            std::size_t centreIdx1 = *cellIt1;
            std::size_t particleIdx1 = centreIdx1 / this->numInteractionCentres;
            std::size_t centre1 = centreIdx1 % this->numInteractionCentres;
            const auto &pos1 = this->absoluteInteractionCentres[centreIdx1];
            const auto &orientation1 = this->shapeOrientations[particleIdx1];
            const auto *data1 = this->getShapeDataPtr(particleIdx1);

            // Overlaps within the cell
            HardcodedTranslation noTranslation({});
            for (auto cellIt2 = std::next(cellIt1); cellIt2 != cellView.end(); cellIt2++) {
                std::size_t centreIdx2 = *cellIt2;
                std::size_t particleIdx2 = centreIdx2 / this->numInteractionCentres;
                if (particleIdx1 == particleIdx2)
                    continue;
                std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
                const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                const auto &orientation2 = this->shapeOrientations[particleIdx2];
                const auto *data2 = this->getShapeDataPtr(particleIdx2);
                if (interaction.overlapBetween(pos1, orientation1, data1, centre1,
                                               pos2, orientation2, data2, centre2,
                                               noTranslation))
                {
                    if (earlyExit) return 1;
                    overlapsCounted++;
                }
            }

            // Overlaps with other cells (but only with a half to avoid redundant checks)
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(coord, true)) {
                HardcodedTranslation cellTrans(cell.getTranslation());
                for (auto centreIdx2 : cell.getNeighbours()) { // NOLINT(readability-use-anyofallof)
                    std::size_t particleIdx2 = centreIdx2 / this->numInteractionCentres;
                    if (particleIdx1 == particleIdx2)
                        continue;
                    std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
                    const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                    const auto &orientation2 = this->shapeOrientations[particleIdx2];
                    const auto *data2 = this->getShapeDataPtr(particleIdx2);
                    if (interaction.overlapBetween(pos1, orientation1, data1, centre1,
                                                   pos2, orientation2, data2, centre2,
                                                   cellTrans))
                    {
                        if (earlyExit) return 1;
                        overlapsCounted++;
                    }
                }
            }
        }
    }

    return overlapsCounted;
}

std::size_t Packing::countTotalOverlaps(const Interaction &interaction, bool earlyExit) const {
    std::size_t overlapsCounted{};

    if (this->neighbourGrid.has_value()) {
        std::atomic<bool> overlapFound = false;
        auto cellDivisions = this->neighbourGrid->getCellDivisions();
        #pragma omp parallel for collapse(3) default(none) shared(overlapFound, interaction, cellDivisions) \
                firstprivate(earlyExit) reduction(+ : overlapsCounted) num_threads(this->scalingThreads)
        for (std::size_t i = 0; i < cellDivisions[0]; i++) {
            for (std::size_t j = 0; j < cellDivisions[1]; j++)  {
                for (std::size_t k = 0; k < cellDivisions[2]; k++) {
                    if (earlyExit && overlapFound.load(std::memory_order_relaxed))
                        continue;

                    std::size_t ngCellOverlaps = this->countTotalOverlapsNGCellHelper({i, j, k}, interaction,
                                                                                      earlyExit);
                    if (earlyExit && ngCellOverlaps > 0)
                        overlapFound.store(true, std::memory_order_relaxed);

                    overlapsCounted += ngCellOverlaps;
                }
            }
        }

        if (earlyExit && overlapsCounted)
            return 1;
    } else {
        for (std::size_t i{}; i < this->size(); i++) {
            for (std::size_t j = i + 1; j < this->size(); j++) {
                std::size_t particleOverlaps = this->countOverlapsBetweenParticlesWithoutNG(i, j, interaction,
                                                                                            earlyExit);
                if (earlyExit && particleOverlaps > 0)
                    return particleOverlaps;

                overlapsCounted += particleOverlaps;
            }
        }
    }

    return overlapsCounted + this->countWallOverlaps(interaction, earlyExit);
}

std::size_t Packing::countWallOverlaps(const Interaction &interaction, bool earlyExit) const {
    if (!this->hasAnyWalls)
        return 0;

    std::atomic<bool> overlapFound = false;
    std::size_t wallOverlaps{};
    #pragma omp parallel for default(none) shared(overlapFound, interaction) firstprivate(earlyExit) \
            reduction(+ : wallOverlaps) num_threads(this->scalingThreads)
    for (std::size_t i = 0; i < size(); i++) {
        if (earlyExit && overlapFound.load(std::memory_order_relaxed))
            continue;

        std::size_t particleWallOverlaps = countParticleWallOverlaps(i, interaction, earlyExit);
        if (earlyExit && particleWallOverlaps > 0)
            overlapFound.store(true, std::memory_order_relaxed);

        wallOverlaps += particleWallOverlaps;
    }
    return wallOverlaps;
}

std::size_t Packing::countOverlapsBetweenParticlesWithoutNG(std::size_t tempParticleIdx, std::size_t anotherParticleIdx,
                                                            const Interaction &interaction, bool earlyExit) const
{
    std::size_t overlapsCounted{};

    if (this->numInteractionCentres == 0) {
        if (interaction.overlapBetween(this->shapePositions[tempParticleIdx],
                                       this->shapeOrientations[tempParticleIdx],
                                       this->getShapeDataPtr(tempParticleIdx),
                                       0,
                                       this->shapePositions[anotherParticleIdx],
                                       this->shapeOrientations[anotherParticleIdx],
                                       this->getShapeDataPtr(anotherParticleIdx),
                                       0,
                                       *this->bc))
        {
            if (earlyExit) return 1;
            overlapsCounted++;
        }
    } else {
        for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
            std::size_t centreIdx1 = tempParticleIdx * this->numInteractionCentres + centre1;
            const auto &pos1 = this->absoluteInteractionCentres[centreIdx1];
            const auto &orientation1 = this->shapeOrientations[tempParticleIdx];
            const auto *data1 = this->getShapeDataPtr(tempParticleIdx);
            for (std::size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                std::size_t centreIdx2 = anotherParticleIdx * this->numInteractionCentres + centre2;
                const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                const auto &orientation2 = this->shapeOrientations[anotherParticleIdx];
                const auto *data2 = this->getShapeDataPtr(anotherParticleIdx);
                if (interaction.overlapBetween(pos1, orientation1, data1, centre1,
                                               pos2, orientation2, data2, centre2,
                                               *this->bc))
                {
                    if (earlyExit) return 1;
                    overlapsCounted++;
                }
            }
        }
    }

    return overlapsCounted;
}

std::size_t Packing::countInteractionCentreOverlapsWithNG(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                                          std::size_t centre, const Interaction &interaction,
                                                          bool earlyExit) const
{
    Expects(this->neighbourGrid.has_value());

    std::size_t overlapsCounted{};

    std::size_t centreIdx1 = tempParticleIdx * this->numInteractionCentres + centre;
    auto pos1 = this->absoluteInteractionCentres[centreIdx1];
    const auto &orientation1 = this->shapeOrientations[tempParticleIdx];
    const auto *data1 = this->getShapeDataPtr(tempParticleIdx);
    for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos1)) {
        HardcodedTranslation cellTranslation(cell.getTranslation());
        for (auto centreIdx2 : cell.getNeighbours()) { // NOLINT(readability-use-anyofallof)
            std::size_t j = centreIdx2 / this->numInteractionCentres;
            if (j == originalParticleIdx)
                continue;
            std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
            const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
            const auto &orientation2 = this->shapeOrientations[j];
            const auto *data2 = this->getShapeDataPtr(j);
            if (interaction.overlapBetween(pos1, orientation1, data1, centre,
                                           pos2, orientation2, data2, centre2,
                                           cellTranslation))
            {
                if (earlyExit) return 1;
                overlapsCounted++;
            }
        }
    }

    return overlapsCounted;
}

double Packing::calculateParticleEnergy(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                        const Interaction &interaction) const
{
    Expects(originalParticleIdx < this->size());
    if (!interaction.hasSoftPart())
        return 0;

    double energy{};
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            const auto &pos = this->shapePositions[tempParticleIdx];
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos)) {
                HardcodedTranslation cellTranslation(cell.getTranslation());
                for (auto j : cell.getNeighbours()) {
                    if (originalParticleIdx == j)
                        continue;
                    energy += interaction.calculateEnergyBetween(this->shapePositions[tempParticleIdx],
                                                                 this->shapeOrientations[tempParticleIdx],
                                                                 this->getShapeDataPtr(tempParticleIdx),
                                                                 0,
                                                                 this->shapePositions[j],
                                                                 this->shapeOrientations[j],
                                                                 this->getShapeDataPtr(j),
                                                                 0,
                                                                 cellTranslation);
                }
            }
        } else {
            for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
                energy += calculateInteractionCentreEnergyWithNG(originalParticleIdx, tempParticleIdx, centre1,
                                                                 interaction);
            }
        }
    } else {
        for (std::size_t j{}; j < this->size(); j++) {
            if (originalParticleIdx == j)
                continue;
            energy += this->calculateEnergyBetweenParticlesWithoutNG(tempParticleIdx, j, interaction);
        }
    }
    return energy;
}

double Packing::getTotalEnergy(const Interaction &interaction) const {
    if (!interaction.hasSoftPart())
        return 0;

    double energy{};
    if (this->neighbourGrid.has_value()) {
        auto cellDivisions = this->neighbourGrid->getCellDivisions();
        #pragma omp parallel for collapse(3) default(none) shared(interaction, cellDivisions) reduction(+:energy) \
                num_threads(this->scalingThreads)
        for (std::size_t i = 0; i < cellDivisions[0]; i++)
            for (std::size_t j = 0; j < cellDivisions[1]; j++)
                for (std::size_t k = 0; k < cellDivisions[2]; k++)
                    energy += this->getTotalEnergyNGCellHelper({i, j, k}, interaction);
    } else {
        for (std::size_t i{}; i < this->size(); i++)
            for (std::size_t j = i + 1; j < this->size(); j++)
                energy += this->calculateEnergyBetweenParticlesWithoutNG(i, j, interaction);
    }
    return energy;
}

double Packing::calculateEnergyBetweenParticlesWithoutNG(std::size_t tempParticleIdx, std::size_t anotherParticleIdx,
                                                         const Interaction &interaction) const
{
    double energy = 0;
    if (this->numInteractionCentres == 0) {
        energy += interaction.calculateEnergyBetween(this->shapePositions[tempParticleIdx],
                                                     this->shapeOrientations[tempParticleIdx],
                                                     this->getShapeDataPtr(tempParticleIdx),
                                                     0,
                                                     this->shapePositions[anotherParticleIdx],
                                                     this->shapeOrientations[anotherParticleIdx],
                                                     this->getShapeDataPtr(anotherParticleIdx),
                                                     0,
                                                     *this->bc);
    } else {
        for (size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
            std::size_t centreIdx1 = tempParticleIdx * this->numInteractionCentres + centre1;
            const auto &pos1 = this->absoluteInteractionCentres[centreIdx1];
            const auto &orientation1 = this->shapeOrientations[tempParticleIdx];
            const auto *data1 = this->getShapeDataPtr(tempParticleIdx);
            for (size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                std::size_t centreIdx2 = anotherParticleIdx * this->numInteractionCentres + centre2;
                const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                const auto &orientation2 =  this->shapeOrientations[anotherParticleIdx];
                const auto *data2 = this->getShapeDataPtr(anotherParticleIdx);
                energy += interaction.calculateEnergyBetween(pos1, orientation1, data1, centre1,
                                                             pos2, orientation2, data2, centre2,
                                                             *this->bc);
            }
        }
    }
    return energy;
}

double Packing::calculateInteractionCentreEnergyWithNG(size_t originalParticleIdx, std::size_t tempParticleIdx,
                                                       std::size_t centre, const Interaction &interaction) const
{
    Expects(this->neighbourGrid.has_value());

    double energy{};

    std::size_t centreIdx1 = tempParticleIdx * this->numInteractionCentres + centre;
    auto pos1 = this->absoluteInteractionCentres[centreIdx1];
    const auto &orientation1 = this->shapeOrientations[tempParticleIdx];
    const auto *data1 = this->getShapeDataPtr(tempParticleIdx);
    for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos1)) {
        HardcodedTranslation cellTranslation(cell.getTranslation());
        for (auto centreIdx2 : cell.getNeighbours()) {
            size_t j = centreIdx2 / this->numInteractionCentres;
            if (j == originalParticleIdx)
                continue;
            size_t centre2 = centreIdx2 % this->numInteractionCentres;
            const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
            const auto &orientation2 = this->shapeOrientations[j];
            const auto *data2 = this->getShapeDataPtr(j);
            energy += interaction.calculateEnergyBetween(pos1, orientation1, data1, centre,
                                                         pos2, orientation2, data2, centre2,
                                                         cellTranslation);
        }
    }
    return energy;
}

double Packing::getTotalEnergyNGCellHelper(const std::array<std::size_t, 3> &coord,
                                           const Interaction &interaction) const
{
    double energy{};
    if (this->numInteractionCentres == 0) {
        const auto &cellView = this->neighbourGrid->getCell(coord);
        for (auto cellIt1 = cellView.begin(); cellIt1 != cellView.end(); cellIt1++) {
            std::size_t particleIdx1 = *cellIt1;
            const auto &pos1 = this->shapePositions[particleIdx1];
            const auto &orientation1 = this->shapeOrientations[particleIdx1];
            const auto *data1 = this->getShapeDataPtr(particleIdx1);

            // Energy within the cell
            HardcodedTranslation noTranslation({});
            for (auto cellIt2 = std::next(cellIt1); cellIt2 != cellView.end(); cellIt2++) {
                std::size_t particleIdx2 = *cellIt2;
                const auto &pos2 = this->shapePositions[particleIdx2];
                const auto &orientation2 = this->shapeOrientations[particleIdx2];
                const auto *data2 = this->getShapeDataPtr(particleIdx2);
                energy += interaction.calculateEnergyBetween(pos1, orientation1, data1, 0,
                                                             pos2, orientation2, data2, 0,
                                                             noTranslation);
            }

            // Energy with other cells (but only with a half to avoid double calculations)
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(coord, true)) {
                HardcodedTranslation cellTranslation(cell.getTranslation());
                for (auto particleIdx2 : cell.getNeighbours()) { // NOLINT(readability-use-anyofallof)
                    const auto &pos2 = this->shapePositions[particleIdx2];
                    const auto &orientation2 = this->shapeOrientations[particleIdx2];
                    const auto *data2 = this->getShapeDataPtr(particleIdx2);
                    energy += interaction.calculateEnergyBetween(pos1, orientation1, data1, 0,
                                                                 pos2, orientation2, data2, 0,
                                                                 cellTranslation);
                }
            }
        }
    } else {
        const auto &cellView = this->neighbourGrid->getCell(coord);
        for (auto cellIt1 = cellView.begin(); cellIt1 != cellView.end(); cellIt1++) {
            std::size_t centreIdx1 = *cellIt1;
            std::size_t particleIdx1 = centreIdx1 / this->numInteractionCentres;
            std::size_t centre1 = centreIdx1 % this->numInteractionCentres;
            const auto &pos1 = this->absoluteInteractionCentres[centreIdx1];
            const auto &orientation1 = this->shapeOrientations[particleIdx1];
            const auto *data1 = this->getShapeDataPtr(particleIdx1);

            // Energy within the cell
            HardcodedTranslation noTranslation({});
            for (auto cellIt2 = std::next(cellIt1); cellIt2 != cellView.end(); cellIt2++) {
                std::size_t centreIdx2 = *cellIt2;
                std::size_t particleIdx2 = centreIdx2 / this->numInteractionCentres;
                if (particleIdx1 == particleIdx2)
                    continue;
                std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
                const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                const auto &orientation2 = this->shapeOrientations[particleIdx2];
                const auto *data2 = this->getShapeDataPtr(particleIdx2);
                energy += interaction.calculateEnergyBetween(pos1, orientation1, data1, centre1,
                                                             pos2, orientation2, data2, centre2,
                                                             noTranslation);
            }

            // Energy with other cells (but only with a half to avoid double calculations)
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(coord, true)) {
                HardcodedTranslation cellTranslation(cell.getTranslation());
                for (auto centreIdx2 : cell.getNeighbours()) { // NOLINT(readability-use-anyofallof)
                    std::size_t particleIdx2 = centreIdx2 / this->numInteractionCentres;
                    if (particleIdx1 == particleIdx2)
                        continue;
                    std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
                    const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                    const auto &orientation2 = this->shapeOrientations[particleIdx2];
                    const auto *data2 = this->getShapeDataPtr(particleIdx2);
                    energy += interaction.calculateEnergyBetween(pos1, orientation1, data1, centre1,
                                                                 pos2, orientation2, data2, centre2,
                                                                 cellTranslation);
                }
            }
        }
    }

    return energy;
}

double Packing::getParticleEnergyFluctuations(const Interaction &interaction) const {
    if (!interaction.hasSoftPart())
        return 0;

    double energySum{};
    double energySum2{};
    for (std::size_t i{}; i < this->size(); i++) {
        double energy = this->calculateParticleEnergy(i, i, interaction);
        energySum += energy;
        energySum2 += energy*energy;
    }

    double N = this->size();
    double doubleEnergy = std::sqrt(energySum2/(N-1) - std::pow(energySum, 2)/N/(N - 1));
    return doubleEnergy / 2;    // We divide by 2, because each interaction was counted twice
}

void Packing::rebuildNeighbourGrid() {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    double cellSize = this->interactionRange;
    // linearSize/cbrt(size()) gives 1 cell per particle, factor 1/5 empirically gives best times
    double minCellSize = std::cbrt(this->getVolume() / this->size()) / 5;
    if (this->interactionRange < minCellSize)
        cellSize = minCellSize;

    // Less than 4 cells in line is redundant, because everything always would be neighbour
    auto boxHeights = this->box.getHeights();
    if (cellSize * 4 > *std::max_element(boxHeights.begin(), boxHeights.end())) {
        this->neighbourGrid = std::nullopt;
        return;
    }

    // If minCellSize makes the cell larger than the box (for example for very few particles in a box very elongated in
    // 2 directions and very narrow in the 3rd one), abort creating NG
    static constexpr double CELL_SIZE_EPSILON = 1 + 1e-12;
    if (cellSize * CELL_SIZE_EPSILON > *std::min_element(boxHeights.begin(), boxHeights.end())) {
        this->neighbourGrid = std::nullopt;
        return;
    }

    std::size_t totalInteractionCentres{};
    if (this->numInteractionCentres == 0)
        totalInteractionCentres = this->size();
    else
        totalInteractionCentres = this->numInteractionCentres*this->size();

    if (!this->neighbourGrid.has_value())
        this->neighbourGrid = NeighbourGrid(this->box, cellSize, totalInteractionCentres);
    else
        this->neighbourGridResizes += this->neighbourGrid->resize(this->box, cellSize);

    this->addInteractionCentresToNeighbourGrid();

    this->neighbourGridRebuilds++;
    auto end = high_resolution_clock::now();
    this->neighbourGridRebuildMicroseconds += duration<double, std::micro>(end - start).count();
}

void Packing::addInteractionCentresToNeighbourGrid() {
    if (numInteractionCentres == 0) {
        std::vector<std::size_t> cellNos(size());

        #pragma omp parallel for default(none) shared(cellNos) num_threads(this->scalingThreads)
        for (std::size_t particleIdx = 0; particleIdx < size(); particleIdx++)
            cellNos[particleIdx] = neighbourGrid->positionToCellNo(shapePositions[particleIdx]);

        for (std::size_t particleIdx{}; particleIdx < size(); particleIdx++)
            neighbourGrid->add(particleIdx, cellNos[particleIdx]);
    } else {
        std::vector<std::size_t> cellNos(size() * numInteractionCentres);

        #pragma omp parallel for default(none) shared(cellNos) num_threads(this->scalingThreads)
        for (std::size_t particleIdx = 0; particleIdx < size(); particleIdx++) {
            for (size_t i{}; i < numInteractionCentres; i++) {
                std::size_t centreIdx = particleIdx * numInteractionCentres + i;
                cellNos[centreIdx] = neighbourGrid->positionToCellNo(absoluteInteractionCentres[centreIdx]);
            }
        }

        for (std::size_t particleIdx{}; particleIdx < size(); particleIdx++) {
            for (size_t i{}; i < numInteractionCentres; i++) {
                std::size_t centreIdx = particleIdx * numInteractionCentres + i;
                neighbourGrid->add(centreIdx, cellNos[centreIdx]);
            }
        }
    }
}

void Packing::setupForInteraction(const Interaction &interaction) {
    // TODO: correct handling of shape data
    Expects(interaction.getShapeDataSize() == this->shapeDataSize);

    this->interactionRange = interaction.getRangeRadius();
    this->numInteractionCentres = interaction.getInteractionCentres().size();
    this->interactionCentres.clear();
    this->absoluteInteractionCentres.clear();
    if (this->numInteractionCentres > 0) {
        // Takes into account temp shapes at the back
        this->interactionCentres.reserve(this->shapePositions.size() * this->numInteractionCentres);
        this->absoluteInteractionCentres.resize(this->shapePositions.size() * this->numInteractionCentres);
        auto centres = interaction.getInteractionCentres();
        // Here, also take temp shape into account (thus this->shapePositions.size(), not this->size())
        for (std::size_t i{}; i < this->shapePositions.size(); i++) {
            const auto &rot = this->shapeOrientations[i];
            for (const auto &centre: centres)
                this->interactionCentres.emplace_back(rot * centre);
        }
        this->recalculateAbsoluteInteractionCentres();
    }
    this->rebuildNeighbourGrid();

    if (this->overlapCounting)
        this->numOverlaps = this->countTotalOverlaps(interaction, false);
}

double Packing::getVolume() const {
    return std::abs(this->box.getVolume());
}

void Packing::store(std::ostream &out, const std::map<std::string, std::string> &auxInfo) const {
    RamsnapWriter writer;
    writer.write(out, *this, auxInfo);
}

std::map<std::string, std::string> Packing::restore(std::istream &in, const Interaction &interaction) {
    RamsnapReader reader;
    return reader.read(in, *this, interaction);
}

void Packing::resetCounters() {
    this->neighbourGridRebuilds = 0;
    this->neighbourGridResizes = 0;
    this->neighbourGridRebuildMicroseconds = 0;
}

std::ostream &operator<<(std::ostream &out, const Packing &packing) {
    out << "Packing {" << std::endl;
    out << "  box: {" << packing.box.getDimensions() << "}," << std::endl;
    out << "  particles (" << packing.size() << "): {" << std::endl;
    for (const auto &shape : packing)
        out << "    " << shape << std::endl;
    out << "  }" << std::endl;
    out << "}";
    return out;
}

std::size_t Packing::getShapesMemoryUsage() const {
    std::size_t bytes{};
    bytes += get_vector_memory_usage(this->shapePositions);
    bytes += get_vector_memory_usage(this->lastShapePositions);
    bytes += get_vector_memory_usage(this->shapeOrientations);
    bytes += get_vector_memory_usage(this->shapeDatas);
    bytes += get_vector_memory_usage(this->interactionCentres);
    bytes += get_vector_memory_usage(this->absoluteInteractionCentres);
    return bytes;
}

std::size_t Packing::getNeighbourGridMemoryUsage() const {
    std::size_t bytes{};
    if (this->neighbourGrid.has_value())
        bytes += this->neighbourGrid->getMemoryUsage();
    if (this->tempNeighbourGrid.has_value())
        bytes += this->tempNeighbourGrid->getMemoryUsage();
    return bytes;
}

double Packing::getAverageNumberOfNeighbours() const {
    if (!this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            return static_cast<double>(this->size() - 1ul);
        else
            return static_cast<double>(this->size()*this->numInteractionCentres - 1ul);
    }

    if (this->numInteractionCentres == 0) {
        std::size_t numNeighbours{};
        for (std::size_t i{}; i < this->size(); i++) {
            const auto &pos = this->shapePositions[i];
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos))
                for (auto j : cell.getNeighbours())
                    if (i != j)
                        numNeighbours++;
        }
        return static_cast<double>(numNeighbours) / static_cast<double>(this->size());
    } else {
        std::size_t numNeighbours{};
        for (std::size_t centreIdx1{}; centreIdx1 < this->size()*this->numInteractionCentres; centreIdx1++) {
            std::size_t particle1 = centreIdx1 / this->numInteractionCentres;
            auto pos1 = this->absoluteInteractionCentres[centreIdx1];
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos1)) {
                for (auto centreIdx2 : cell.getNeighbours()) { // NOLINT(readability-use-anyofallof)
                    std::size_t particle2 = centreIdx2 / this->numInteractionCentres;
                    if (particle2 != particle1)
                        numNeighbours++;
                }
            }
        }
        return static_cast<double>(numNeighbours) / static_cast<double>(this->size()*this->numInteractionCentres);
    }
}

void Packing::recalculateAbsoluteInteractionCentres() {
    #pragma omp parallel for default(none) num_threads(this->scalingThreads)
    for (std::size_t particleIdx = 0; particleIdx < this->size(); particleIdx++)
        this->recalculateAbsoluteInteractionCentres(particleIdx);
}

void Packing::recalculateAbsoluteInteractionCentres(std::size_t particleIdx) {
    for (std::size_t centre{}; centre < this->numInteractionCentres; centre++) {
        std::size_t centreIdx = particleIdx * this->numInteractionCentres + centre;
        auto pos = this->shapePositions[particleIdx] + this->interactionCentres[centreIdx];
        this->absoluteInteractionCentres[centreIdx] = pos + this->bc->getCorrection(pos);
    }
}

void Packing::toggleOverlapCounting(bool countOverlaps, const Interaction &interaction) {
    this->overlapCounting = countOverlaps;
    if (this->overlapCounting)
        this->numOverlaps = this->countTotalOverlaps(interaction, false);
}

std::size_t Packing::getCachedNumberOfOverlaps() const {
    if (this->overlapCounting)
        return this->numOverlaps;

    throw std::runtime_error("Packing: overlap counting is toggled false; number of overlaps is not cached");
}

void Packing::resetNGRaceConditionSanitizer() {
    if (this->neighbourGrid.has_value())
        this->neighbourGrid->resetRaceConditionSanitizer();
}

bool Packing::areShapesWithinBox(const std::vector<Shape> &shapes, const TriclinicBox &box) {
    for (const auto &shape : shapes)
        for (auto posCoord: box.absoluteToRelative(shape.getPosition()))
            if (posCoord < 0 || posCoord >= 1)
                return false;
    return true;
}

void Packing::toggleWall(std::size_t wallAxis, bool trueOrFalse) {
    Expects(wallAxis < 3);
    this->hasWall[wallAxis] = trueOrFalse;
    this->hasAnyWalls = std::find(this->hasWall.begin(), this->hasWall.end(), true) != this->hasWall.end();
}

void Packing::toggleWalls(std::array<bool, 3> axisWalls) {
    this->hasWall = axisWalls;
    this->hasAnyWalls = std::find(this->hasWall.begin(), this->hasWall.end(), true) != this->hasWall.end();
}

std::size_t Packing::countParticleWallOverlaps(std::size_t particleIdx, const Interaction &interaction,
                                               bool earlyExit) const
{
    std::size_t countedOverlaps{};
    const auto &boxSides = this->box.getSides();
    Vector<3> boxOrigin{};
    Vector<3> furtherBoxOrigin = std::accumulate(boxSides.begin(), boxSides.end(), boxOrigin);
    double halfTotalRangeRadius = interaction.getTotalRangeRadius() / 2;

    for (std::size_t i{}; i < 3; i++) {
        if (!this->hasWall[i])
            continue;

        std::size_t nextCoord = (i + 1) % 3;
        std::size_t nextNextCoord = (i + 2) % 3;
        Vector<3> wallVector = (boxSides[nextCoord] ^ boxSides[nextNextCoord]).normalized();
        const auto &shapePos = this->shapePositions[particleIdx];
        const auto &shapeRot = this->shapeOrientations[particleIdx];
        const auto *shapeData = this->getShapeDataPtr(particleIdx);
        if (shapePos * wallVector < halfTotalRangeRadius) {
            // Nearer wall
            if (this->numInteractionCentres == 0) {
                if (interaction.overlapWithWall(shapePos, shapeRot, shapeData, 0, boxOrigin, wallVector)) {
                    if (earlyExit) return 1;
                    countedOverlaps++;
                }
            } else {
                std::size_t centreIdxOrigin = particleIdx * this->numInteractionCentres;
                for (std::size_t centre{}; centre < this->numInteractionCentres; centre++) {
                    Vector<3> centrePos = shapePos + this->interactionCentres[centreIdxOrigin + centre];
                    if (interaction.overlapWithWall(centrePos, shapeRot, shapeData, centre, boxOrigin, wallVector)) {
                        if (earlyExit) return 1;
                        countedOverlaps++;
                    }
                }
            }
        } else if ((furtherBoxOrigin - shapePos) * wallVector < halfTotalRangeRadius) {
            // Further wall
            if (this->numInteractionCentres == 0) {
                if (interaction.overlapWithWall(shapePos, shapeRot, shapeData, 0, furtherBoxOrigin, -wallVector)) {
                    if (earlyExit) return 1;
                    countedOverlaps++;
                }
            } else {
                std::size_t centreIdxOrigin = particleIdx * this->numInteractionCentres;
                for (std::size_t centre{}; centre < this->numInteractionCentres; centre++) {
                    Vector<3> centrePos = shapePos + this->interactionCentres[centreIdxOrigin + centre];
                    if (interaction.overlapWithWall(centrePos, shapeRot, shapeData, centre, furtherBoxOrigin,
                                                    -wallVector))
                    {
                        if (earlyExit) return 1;
                        countedOverlaps++;
                    }
                }
            }
        }
    }

    return countedOverlaps;
}

std::vector<Vector<3>> Packing::dumpNamedPoints(const ShapeGeometry &geometry, const std::string &pointName) const {
    std::vector<Vector<3>> namedPoints;
    namedPoints.reserve(this->size());
    Vector<3> alignedNamedPoint = geometry.getNamedPoint(pointName);
    auto namedPointsCalculator = [alignedNamedPoint](const Shape &shape) {
        return shape.getPosition() + shape.getOrientation() * alignedNamedPoint;
    };
    std::transform(this->begin(), this->end(), std::back_inserter(namedPoints), namedPointsCalculator);
    return namedPoints;
}

bool Packing::isBoxUpscaled(const TriclinicBox &oldBox, const TriclinicBox &newBox) {
    const auto &oldSides = oldBox.getSides();
    const auto &newSides = newBox.getSides();

    constexpr double EPSILON = 100*std::numeric_limits<double>::epsilon();
    for (std::size_t i{}; i < 3; i++) {
        const auto &oldSide = oldSides[i];
        const auto &newSide = newSides[i];

        double dot = oldSide*newSide;
        double norm2Old = oldSide.norm2();
        double norm2New = newSide.norm2();
        // For now, we can handle only box transformations without sheering and rotations - exit if box vectors are not
        // co-linear
        if (std::abs(dot*dot/norm2Old/norm2New - 1) > EPSILON)
            return false;
        // Factor < 1 may lead to an overlap - box is not only upscaled
        if (dot*dot/norm2Old/norm2Old < 1 + EPSILON)
            return false;
    }
    return true;
}

std::size_t Packing::renormalizeOrientations(const Interaction &interaction, bool allowOverlaps) {
    if (allowOverlaps) {
        auto newShapes = std::vector<Shape>(this->begin(), this->end());
        #pragma omp parallel for shared(newShapes) default(none) num_threads(this->scalingThreads)
        for (std::size_t i = 0; i < this->size(); i++)
            Packing::fixRotationMatrix(this->shapeOrientations[i]);
        this->reset(std::move(newShapes), this->box, interaction);
        return 0;
    }

    // For a good measure, reset NG sanitizer - it may have not been reset after last operation
    this->resetNGRaceConditionSanitizer();

    auto centres = interaction.getInteractionCentres();
    std::size_t rejectionCounter{};
    // We can't parallelize this loop because of the race condition on neighbour grid cells
    for (std::size_t particleIdx = 0; particleIdx < this->size(); particleIdx++) {
        // threadId should be 0 (master), but fetch explicitly if somehow this function is run from a different thread
        auto threadId = OMP_THREAD_ID;
        std::size_t tempParticleIdx = this->size() + threadId;

        this->tryOrientationFix(particleIdx, centres);

        if (this->calculateMoveOverlapEnergy(particleIdx, tempParticleIdx, interaction) > 0)
            rejectionCounter++;
        else
            this->acceptRotation();
    }

    return rejectionCounter;
}

void Packing::fixRotationMatrix(Matrix<3, 3> &rotation) {
    // Iterative algorithm from https://math.stackexchange.com/questions/3292034/normalizing-a-rotation-matrix
    // Do 3 iterations, however usually 1 is enough
    for (std::size_t i{}; i < 3; i++)
        rotation = 1.5 * rotation - 0.5 * rotation * rotation.transpose() * rotation;
}

void Packing::tryOrientationFix(std::size_t particleIdx, const std::vector<Vector<3>> &centres) {
    // threadId should be 0 (master), but fetch explicitly if somehow this function is run from a different thread
    auto threadId = OMP_THREAD_ID;
    std::size_t tempParticleIdx = this->size() + threadId;
    this->copyShape(particleIdx, tempParticleIdx);
    this->lastAlteredParticleIdx[threadId] = particleIdx;

    auto &tempRot = this->shapeOrientations[tempParticleIdx];
    Packing::fixRotationMatrix(tempRot);

    if (this->numInteractionCentres > 0) {
        std::size_t tempOrigin = (this->size() + threadId) * this->numInteractionCentres;
        for (std::size_t centreI{}; centreI < centres.size(); centreI++)
            this->interactionCentres[tempOrigin + centreI] = tempRot * centres[centreI];
        this->recalculateAbsoluteInteractionCentres(tempParticleIdx);
    }
}

void Packing::copyShape(std::size_t fromIdx, std::size_t toIdx) {
    this->shapePositions[toIdx] = this->shapePositions[fromIdx];
    this->shapeOrientations[toIdx] = this->shapeOrientations[fromIdx];

    auto fromBegin = this->shapeDatas.begin() + static_cast<std::ptrdiff_t>(this->shapeDataSize * fromIdx);
    auto fromEnd = fromBegin + static_cast<std::ptrdiff_t>(this->shapeDataSize);
    auto toBegin = this->shapeDatas.begin() + static_cast<std::ptrdiff_t>(this->shapeDataSize * toIdx);
    std::copy(fromBegin, fromEnd, toBegin);
}

void Packing::translateShapeWithoutInteractionCenters(std::size_t idx, const Vector<3> &translation) {
    auto &shapePos = this->shapePositions[idx];
    shapePos += translation;
    shapePos += this->bc->getCorrection(shapePos);
}

void Packing::rotateShapeWithoutInteractionCenters(std::size_t idx, const Matrix<3, 3> &rotation) {
    auto &shapeRot = this->shapeOrientations[idx];
    shapeRot = rotation * shapeRot;
}

const std::byte *Packing::getShapeDataPtr(std::size_t particleIdx) const {
    return this->shapeDatas.data() + static_cast<std::ptrdiff_t>(particleIdx * this->shapeDataSize);
}

