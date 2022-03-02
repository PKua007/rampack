//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cmath>
#include <ostream>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <atomic>

#include "Packing.h"
#include "utils/Assertions.h"
#include "utils/Utils.h"


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

Packing::Packing(TriclinicBox box, std::vector<Shape> shapes,
                 std::unique_ptr<BoundaryConditions> bc, const Interaction &interaction, std::size_t moveThreads,
                 std::size_t scalingThreads)
        : shapes{std::move(shapes)}, box{std::move(box)}, bc{std::move(bc)},
          interactionRange{interaction.getRangeRadius()}
{
    Expects(this->box.getVolume() != 0);
    Expects(this->interactionRange > 0);
    Expects(!this->shapes.empty());

    this->moveThreads = (moveThreads == 0 ? _OMP_MAXTHREADS : moveThreads);
    this->scalingThreads = (scalingThreads == 0 ? _OMP_MAXTHREADS : scalingThreads);

    this->shapes.resize(this->shapes.size() + this->moveThreads);    // temp shapes at the back
    this->lastAlteredParticleIdx.resize(this->moveThreads, 0);
    this->bc->setBox(this->box);
    this->setupForInteraction(interaction);
}

Packing::Packing(std::unique_ptr<BoundaryConditions> bc, std::size_t moveThreads, std::size_t scalingThreads)
        : bc{std::move(bc)}
{
    this->moveThreads = (moveThreads == 0 ? _OMP_MAXTHREADS : moveThreads);
    this->scalingThreads = (scalingThreads == 0 ? _OMP_MAXTHREADS : scalingThreads);

    this->lastAlteredParticleIdx.resize(this->moveThreads, 0);
}

double Packing::tryTranslation(std::size_t particleIdx, Vector<3> translation, const Interaction &interaction,
                               std::optional<ActiveDomain> boundaries)
{
    Expects(particleIdx < this->size());
    Expects(interaction.getRangeRadius() <= this->interactionRange);

    std::size_t tempParticleIdx = this->size() + _OMP_THREAD_ID;
    this->lastAlteredParticleIdx[_OMP_THREAD_ID] = particleIdx;
    this->shapes[tempParticleIdx] = this->shapes[particleIdx];
    this->shapes[tempParticleIdx].translate(translation, *this->bc);

    if (boundaries.has_value() && !boundaries->isInside(this->shapes[tempParticleIdx].getPosition()))
        return std::numeric_limits<double>::infinity();

    if (this->numInteractionCentres != 0) {
        this->prepareTempInteractionCentres(particleIdx);
        this->recalculateAbsoluteInteractionCentres(tempParticleIdx);
    }

    static constexpr double INF = std::numeric_limits<double>::infinity();
    if (interaction.hasHardPart()) {
        if (this->overlapCounting) {
            std::size_t initialOverlaps = this->countParticleOverlaps(particleIdx, particleIdx, interaction, false);
            std::size_t finalOverlaps = this->countParticleOverlaps(particleIdx, tempParticleIdx, interaction, false);
            this->lastOverlapDelta = static_cast<int>(finalOverlaps) - initialOverlaps;
            if (this->lastOverlapDelta < 0)
                return -INF;
            else if (this->lastOverlapDelta > 0)
                return INF;
        } else {
            if (this->countParticleOverlaps(particleIdx, tempParticleIdx, interaction, true) > 0)
                return std::numeric_limits<double>::infinity();
        }
    }

    double initialEnergy = this->calculateParticleEnergy(particleIdx, particleIdx, interaction);
    double finalEnergy = this->calculateParticleEnergy(particleIdx, tempParticleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryRotation(std::size_t particleIdx, const Matrix<3, 3> &rotation, const Interaction &interaction) {
    Expects(particleIdx < this->size());
    Expects(interaction.getRangeRadius() <= this->interactionRange);

    std::size_t tempParticleIdx = this->size() + _OMP_THREAD_ID;
    this->lastAlteredParticleIdx[_OMP_THREAD_ID] = particleIdx;
    this->shapes[tempParticleIdx] = this->shapes[particleIdx];

    this->shapes[tempParticleIdx].rotate(rotation);
    if (this->numInteractionCentres != 0) {
        this->prepareTempInteractionCentres(particleIdx);
        this->rotateTempInteractionCentres(rotation);
        this->recalculateAbsoluteInteractionCentres(tempParticleIdx);
    }

    static constexpr double INF = std::numeric_limits<double>::infinity();
    if (interaction.hasHardPart()) {
        if (this->overlapCounting) {
            std::size_t initialOverlaps = this->countParticleOverlaps(particleIdx, particleIdx, interaction, false);
            std::size_t finalOverlaps = this->countParticleOverlaps(particleIdx, tempParticleIdx, interaction, false);
            this->lastOverlapDelta = static_cast<int>(finalOverlaps) - initialOverlaps;
            if (this->lastOverlapDelta < 0)
                return -INF;
            else if (this->lastOverlapDelta > 0)
                return INF;
        } else {
            if (this->countParticleOverlaps(particleIdx, tempParticleIdx, interaction, true) > 0)
                return std::numeric_limits<double>::infinity();
        }
    }

    double initialEnergy = this->calculateParticleEnergy(particleIdx, particleIdx, interaction);
    double finalEnergy = this->calculateParticleEnergy(particleIdx, tempParticleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryMove(std::size_t particleIdx, const Vector<3> &translation, const Matrix<3, 3> &rotation,
                        const Interaction &interaction, std::optional<ActiveDomain> boundaries)
{
    Expects(particleIdx < this->size());
    Expects(interaction.getRangeRadius() <= this->interactionRange);

    std::size_t tempParticleIdx = this->size() + _OMP_THREAD_ID;
    this->lastAlteredParticleIdx[_OMP_THREAD_ID] = particleIdx;
    this->shapes[tempParticleIdx] = this->shapes[particleIdx];
    this->shapes[tempParticleIdx].translate(translation, *this->bc);

    if (boundaries.has_value() && !boundaries->isInside(this->shapes[tempParticleIdx].getPosition()))
        return std::numeric_limits<double>::infinity();

    this->shapes[tempParticleIdx].rotate(rotation);
    if (this->numInteractionCentres != 0) {
        this->prepareTempInteractionCentres(particleIdx);
        this->rotateTempInteractionCentres(rotation);
        this->recalculateAbsoluteInteractionCentres(tempParticleIdx);
    }

    static constexpr double INF = std::numeric_limits<double>::infinity();
    if (interaction.hasHardPart()) {
        if (this->overlapCounting) {
            std::size_t initialOverlaps = this->countParticleOverlaps(particleIdx, particleIdx, interaction, false);
            std::size_t finalOverlaps = this->countParticleOverlaps(particleIdx, tempParticleIdx, interaction, false);
            this->lastOverlapDelta = static_cast<int>(finalOverlaps) - initialOverlaps;
            if (this->lastOverlapDelta < 0)
                return -INF;
            else if (this->lastOverlapDelta > 0)
                return INF;
        } else {
            if (this->countParticleOverlaps(particleIdx, tempParticleIdx, interaction, true) > 0)
                return std::numeric_limits<double>::infinity();
        }
    }

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

    double initialEnergy = this->getTotalEnergy(interaction);
    this->lastNumOverlaps = this->numOverlaps;

    this->box = newBox;
    this->bc->setBox(this->box);
    for (auto &shape : *this)
        shape.setPosition(this->box.relativeToAbsolute(this->lastBox.absoluteToRelative(shape.getPosition())));
    std::swap(this->neighbourGrid, this->tempNeighbourGrid);
    if (this->numInteractionCentres != 0)
        this->recalculateAbsoluteInteractionCentres();
    this->rebuildNeighbourGrid();

    static constexpr double INF = std::numeric_limits<double>::infinity();
    if (interaction.hasHardPart()) {
        if (this->overlapCounting) {
            this->numOverlaps = this->countTotalOverlaps(interaction, false);
            int overlapDelta = static_cast<int>(this->numOverlaps) - this->lastNumOverlaps;
            if (overlapDelta < 0)
                return -INF;
            else if (overlapDelta > 0)
                return INF;
        } else {
            if (this->countTotalOverlaps(interaction, true) > 0)
                return INF;
        }
    }

    double finalEnergy = this->getTotalEnergy(interaction);
    return finalEnergy - initialEnergy;
}

const Shape &Packing::operator[](std::size_t i) const {
    Expects(i < this->size());
    return this->shapes[i];
}

const Shape &Packing::front() const {
    Expects(!this->empty());
    return this->shapes.front();
}

const Shape &Packing::back() const {
    Expects(!this->empty());
    return this->shapes[this->size() - 1];
}

void Packing::toWolfram(std::ostream &out, const ShapePrinter &shapePrinter) const {
    out << "Graphics3D[{" << std::endl;
    for (std::size_t i{}; i < this->size(); i++) {
        const auto &shape = shapes[i];
        out << shapePrinter.toWolfram(shape);
        if (i != this->size() - 1)
            out << "," << std::endl;
    }
    out << "}]";
}

double Packing::getPackingFraction(double shapeVolume) const {
    Expects(shapeVolume >= 0);
    return this->getNumberDensity() * shapeVolume;
}

double Packing::getNumberDensity() const {
    return this->size() / this->getVolume();
}

void Packing::acceptTranslation() {
    std::size_t lastAlteredIdx = this->lastAlteredParticleIdx[_OMP_THREAD_ID];
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->remove(lastAlteredIdx, this->shapes[lastAlteredIdx].getPosition());
        else
            this->removeInteractionCentresFromNeighbourGrid(lastAlteredIdx);
    }

    this->shapes[lastAlteredIdx].setPosition(this->shapes[this->size() + _OMP_THREAD_ID].getPosition());
    if (this->numInteractionCentres != 0)
        this->acceptTempInteractionCentres();

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->add(lastAlteredIdx, this->shapes[lastAlteredIdx].getPosition());
        else
            this->addInteractionCentresToNeighbourGrid(lastAlteredIdx);
    }

    if (this->overlapCounting)
        this->numOverlaps += this->lastOverlapDelta;
}

void Packing::acceptRotation() {
    std::size_t lastAlteredIdx = this->lastAlteredParticleIdx[_OMP_THREAD_ID];
    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0)
        this->removeInteractionCentresFromNeighbourGrid(lastAlteredIdx);

    this->shapes[lastAlteredIdx].setOrientation(this->shapes[size() + _OMP_THREAD_ID].getOrientation());
    if (this->numInteractionCentres != 0)
        this->acceptTempInteractionCentres();

    if (this->neighbourGrid.has_value() && this->numInteractionCentres != 0)
        this->addInteractionCentresToNeighbourGrid(lastAlteredIdx);

    if (this->overlapCounting)
        this->numOverlaps += this->lastOverlapDelta;
}

void Packing::acceptMove() {
    std::size_t lastAlteredIdx = this->lastAlteredParticleIdx[_OMP_THREAD_ID];
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->remove(lastAlteredIdx, this->shapes[lastAlteredIdx].getPosition());
        else
            this->removeInteractionCentresFromNeighbourGrid(lastAlteredIdx);
    }

    this->shapes[lastAlteredIdx] = this->shapes[this->size() + _OMP_THREAD_ID];
    if (this->numInteractionCentres != 0)
        this->acceptTempInteractionCentres();

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->add(lastAlteredIdx, this->shapes[lastAlteredIdx].getPosition());
        else
            this->addInteractionCentresToNeighbourGrid(lastAlteredIdx);
    }

    if (this->overlapCounting)
        this->numOverlaps += this->lastOverlapDelta;
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
    std::size_t fromOrigin = (this->size() + _OMP_THREAD_ID) * this->numInteractionCentres;
    std::size_t toOrigin = this->lastAlteredParticleIdx[_OMP_THREAD_ID] * this->numInteractionCentres;
    for (std::size_t i{}; i < this->numInteractionCentres; i++) {
        std::size_t toCentreIdx = toOrigin + i;
        std::size_t fromCentreIdx = fromOrigin + i;
        this->interactionCentres[toCentreIdx] = this->interactionCentres[fromCentreIdx];
        this->absoluteInteractionCentres[toCentreIdx] = this->absoluteInteractionCentres[fromCentreIdx];
    }
}

void Packing::prepareTempInteractionCentres(std::size_t particleIdx) {
    std::size_t fromOrigin = particleIdx * this->numInteractionCentres;
    std::size_t toOrigin = (this->size() + _OMP_THREAD_ID) * this->numInteractionCentres;
    for (std::size_t i{}; i < this->numInteractionCentres; i++)
        this->interactionCentres[toOrigin + i] = this->interactionCentres[fromOrigin + i];
}

void Packing::rotateTempInteractionCentres(const Matrix<3, 3> &rotation) {
    std::size_t idxOrigin = (this->size() + _OMP_THREAD_ID) * this->numInteractionCentres;
    for (std::size_t i{}; i < this->numInteractionCentres; i++)
        this->interactionCentres[idxOrigin + i] = rotation * this->interactionCentres[idxOrigin + i];
}

void Packing::revertScaling() {
    for (auto &shape : *this)
        shape.setPosition(this->lastBox.relativeToAbsolute(this->box.absoluteToRelative(shape.getPosition())));

    this->box = this->lastBox;
    this->bc->setBox(this->box);
    std::swap(this->neighbourGrid, this->tempNeighbourGrid);
    if (this->numInteractionCentres != 0)
        this->recalculateAbsoluteInteractionCentres();
    this->numOverlaps = this->lastNumOverlaps;
}

std::size_t Packing::countParticleOverlaps(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                           const Interaction &interaction, bool earlyExit) const
{
    std::size_t overlapsCounted{};

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            Vector<3> pos = this->shapes[tempParticleIdx].getPosition();
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos)) {
                HardcodedTranslation cellTranslation(cell.getTranslation());
                for (auto j: cell.getNeighbours()) {
                    if (originalParticleIdx == j)
                        continue;

                    if (interaction.overlapBetween(this->shapes[tempParticleIdx].getPosition(),
                                                   this->shapes[tempParticleIdx].getOrientation(),
                                                   0,
                                                   this->shapes[j].getPosition(),
                                                   this->shapes[j].getOrientation(),
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
    return overlapsCounted;
}

std::size_t Packing::countTotalOverlapsNGCellHelper(const std::array<std::size_t, 3> &coord,
                                                    const Interaction &interaction, bool earlyExit) const
{
    std::size_t overlapsCounted{};

    if (this->numInteractionCentres == 0) {
        const auto &cellView = this->neighbourGrid->getCell(coord);
        for (auto cellIt1 = cellView.begin(); cellIt1 != cellView.end(); cellIt1++) {
            std::size_t particleIdx1 = *cellIt1;
            const auto &pos1 = this->shapes[particleIdx1].getPosition();
            const auto &orientation1 = this->shapes[particleIdx1].getOrientation();

            // Overlaps within the cell
            HardcodedTranslation noTranslation({});
            for (auto cellIt2 = std::next(cellIt1); cellIt2 != cellView.end(); cellIt2++) {
                std::size_t particleIdx2 = *cellIt2;
                const auto &pos2 = this->shapes[particleIdx2].getPosition();
                const auto &orientation2 = this->shapes[particleIdx2].getOrientation();
                if (interaction.overlapBetween(pos1, orientation1, 0, pos2, orientation2, 0, noTranslation)) {
                    if (earlyExit) return 1;
                    overlapsCounted++;
                }
            }

            // Overlaps with other cells (but only with a half to avoid redundant checks)
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(coord, true)) {
                HardcodedTranslation cellTranslation(cell.getTranslation());
                for (auto particleIdx2 : cell.getNeighbours()) { // NOLINT(readability-use-anyofallof)
                    const auto &pos2 = this->shapes[particleIdx2].getPosition();
                    const auto &orientation2 = this->shapes[particleIdx2].getOrientation();
                    if (interaction.overlapBetween(pos1, orientation1, 0, pos2, orientation2, 0, cellTranslation)) {
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
            const auto &orientation1 = this->shapes[particleIdx1].getOrientation();

            // Overlaps within the cell
            HardcodedTranslation noTranslation({});
            for (auto cellIt2 = std::next(cellIt1); cellIt2 != cellView.end(); cellIt2++) {
                std::size_t centreIdx2 = *cellIt2;
                std::size_t particleIdx2 = centreIdx2 / this->numInteractionCentres;
                if (particleIdx1 == particleIdx2)
                    continue;
                std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
                const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                const auto &orientation2 = this->shapes[particleIdx2].getOrientation();
                if (interaction.overlapBetween(pos1, orientation1, centre1, pos2, orientation2, centre2,
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
                    const auto &orientation2 = this->shapes[particleIdx2].getOrientation();
                    if (interaction.overlapBetween(pos1, orientation1, centre1, pos2, orientation2, centre2,
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
    if (this->neighbourGrid.has_value()) {
        std::atomic<bool> overlapFound = false;
        std::size_t overlapsCounted{};
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

        return overlapsCounted;
    } else {
        std::size_t overlapsCounted{};
        for (std::size_t i{}; i < this->size(); i++) {
            for (std::size_t j = i + 1; j < this->size(); j++) {
                std::size_t particleOverlaps = this->countOverlapsBetweenParticlesWithoutNG(i, j, interaction,
                                                                                            earlyExit);
                if (earlyExit && particleOverlaps > 0)
                    return particleOverlaps;

                overlapsCounted += particleOverlaps;
            }
        }
        return overlapsCounted;
    }
}

std::size_t Packing::countOverlapsBetweenParticlesWithoutNG(std::size_t tempParticleIdx, std::size_t anotherParticleIdx,
                                                            const Interaction &interaction, bool earlyExit) const
{
    std::size_t overlapsCounted{};

    if (this->numInteractionCentres == 0) {
        if (interaction.overlapBetween(this->shapes[tempParticleIdx].getPosition(),
                                       this->shapes[tempParticleIdx].getOrientation(),
                                       0,
                                       this->shapes[anotherParticleIdx].getPosition(),
                                       this->shapes[anotherParticleIdx].getOrientation(),
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
            const auto &orientation1 = this->shapes[tempParticleIdx].getOrientation();
            for (std::size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                std::size_t centreIdx2 = anotherParticleIdx * this->numInteractionCentres + centre2;
                const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                const auto &orientation2 = this->shapes[anotherParticleIdx].getOrientation();
                if (interaction.overlapBetween(pos1, orientation1, centre1, pos2, orientation2, centre2, *this->bc)) {
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
    const auto &orientation1 = this->shapes[tempParticleIdx].getOrientation();
    for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos1)) {
        HardcodedTranslation cellTranslation(cell.getTranslation());
        for (auto centreIdx2 : cell.getNeighbours()) { // NOLINT(readability-use-anyofallof)
            std::size_t j = centreIdx2 / this->numInteractionCentres;
            if (j == originalParticleIdx)
                continue;
            std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
            const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
            const auto &orientation2 = this->shapes[j].getOrientation();
            if (interaction.overlapBetween(pos1, orientation1, centre, pos2, orientation2, centre2, cellTranslation)){
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
            const auto &pos = this->shapes[tempParticleIdx].getPosition();
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos)) {
                HardcodedTranslation cellTranslation(cell.getTranslation());
                for (auto j : cell.getNeighbours()) {
                    if (originalParticleIdx == j)
                        continue;
                    energy += interaction.calculateEnergyBetween(this->shapes[tempParticleIdx].getPosition(),
                                                                 this->shapes[tempParticleIdx].getOrientation(),
                                                                 0,
                                                                 this->shapes[j].getPosition(),
                                                                 this->shapes[j].getOrientation(),
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
        energy += interaction.calculateEnergyBetween(this->shapes[tempParticleIdx].getPosition(),
                                                     this->shapes[tempParticleIdx].getOrientation(),
                                                     0,
                                                     this->shapes[anotherParticleIdx].getPosition(),
                                                     this->shapes[anotherParticleIdx].getOrientation(),
                                                     0,
                                                     *this->bc);
    } else {
        for (size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
            std::size_t centreIdx1 = tempParticleIdx * this->numInteractionCentres + centre1;
            const auto &pos1 = this->absoluteInteractionCentres[centreIdx1];
            const auto &orientation1 = this->shapes[tempParticleIdx].getOrientation();
            for (size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                std::size_t centreIdx2 = anotherParticleIdx * this->numInteractionCentres + centre2;
                const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                const auto &orientation2 =  this->shapes[anotherParticleIdx].getOrientation();
                energy += interaction.calculateEnergyBetween(pos1, orientation1, centre1, pos2, orientation2, centre2,
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
    const auto &orientation1 = this->shapes[tempParticleIdx].getOrientation();
    for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos1)) {
        HardcodedTranslation cellTranslation(cell.getTranslation());
        for (auto centreIdx2 : cell.getNeighbours()) {
            size_t j = centreIdx2 / this->numInteractionCentres;
            if (j == originalParticleIdx)
                continue;
            size_t centre2 = centreIdx2 % this->numInteractionCentres;
            const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
            const auto &orientation2 = this->shapes[j].getOrientation();
            energy += interaction.calculateEnergyBetween(pos1, orientation1, centre, pos2, orientation2, centre2,
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
            const auto &pos1 = this->shapes[particleIdx1].getPosition();
            const auto &orientation1 = this->shapes[particleIdx1].getOrientation();

            // Energy within the cell
            HardcodedTranslation noTranslation({});
            for (auto cellIt2 = std::next(cellIt1); cellIt2 != cellView.end(); cellIt2++) {
                std::size_t particleIdx2 = *cellIt2;
                const auto &pos2 = this->shapes[particleIdx2].getPosition();
                const auto &orientation2 = this->shapes[particleIdx2].getOrientation();
                energy += interaction.calculateEnergyBetween(pos1, orientation1, 0, pos2, orientation2, 0,
                                                             noTranslation);
            }

            // Energy with other cells (but only with a half to avoid double calculations)
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(coord, true)) {
                HardcodedTranslation cellTranslation(cell.getTranslation());
                for (auto particleIdx2 : cell.getNeighbours()) { // NOLINT(readability-use-anyofallof)
                    const auto &pos2 = this->shapes[particleIdx2].getPosition();
                    const auto &orientation2 = this->shapes[particleIdx2].getOrientation();
                    energy += interaction.calculateEnergyBetween(pos1, orientation1, 0, pos2, orientation2, 0,
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
            const auto &orientation1 = this->shapes[particleIdx1].getOrientation();

            // Energy within the cell
            HardcodedTranslation noTranslation({});
            for (auto cellIt2 = std::next(cellIt1); cellIt2 != cellView.end(); cellIt2++) {
                std::size_t centreIdx2 = *cellIt2;
                std::size_t particleIdx2 = centreIdx2 / this->numInteractionCentres;
                if (particleIdx1 == particleIdx2)
                    continue;
                std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
                const auto &pos2 = this->absoluteInteractionCentres[centreIdx2];
                const auto &orientation2 = this->shapes[particleIdx2].getOrientation();
                energy += interaction.calculateEnergyBetween(pos1, orientation1, centre1, pos2, orientation2, centre2,
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
                    const auto &orientation2 = this->shapes[particleIdx2].getOrientation();
                    energy += interaction.calculateEnergyBetween(pos1, orientation1, centre1, pos2, orientation2,
                                                                 centre2, cellTranslation);
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
            cellNos[particleIdx] = neighbourGrid->positionToCellNo(shapes[particleIdx].getPosition());

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
    this->interactionRange = interaction.getRangeRadius();
    this->numInteractionCentres = interaction.getInteractionCentres().size();
    this->interactionCentres.clear();
    this->absoluteInteractionCentres.clear();
    if (this->numInteractionCentres > 0) {
        // Takes into account temp shapes at the back
        this->interactionCentres.reserve(this->shapes.size() * this->numInteractionCentres);
        this->absoluteInteractionCentres.resize(this->shapes.size() * this->numInteractionCentres);
        auto centres = interaction.getInteractionCentres();
        for (const auto &shape : this->shapes)
            for (const auto &centre : centres)
                this->interactionCentres.emplace_back(shape.getOrientation() * centre);
        this->recalculateAbsoluteInteractionCentres();
    }
    this->rebuildNeighbourGrid();
}

double Packing::getVolume() const {
    return std::abs(this->box.getVolume());
}

void Packing::store(std::ostream &out, const std::map<std::string, std::string> &auxInfo) const {
    out << auxInfo.size() << std::endl;
    for (const auto &infoEntry : auxInfo) {
        const auto &key = infoEntry.first;
        const auto &value = infoEntry.second;
        Expects(std::none_of(key.begin(), key.end(), [](char c) { return std::isspace(c); }));
        out << key << " " << value << std::endl;
    }

    out.precision(std::numeric_limits<double>::max_digits10);
    const auto &dimensions = this->box.getDimensions();
    out << dimensions(0, 0) << " " << dimensions(0, 1) << " " << dimensions(0, 2) << " ";
    out << dimensions(1, 0) << " " << dimensions(1, 1) << " " << dimensions(1, 2) << " ";
    out << dimensions(2, 0) << " " << dimensions(2, 1) << " " << dimensions(2, 2) << std::endl;
    out << this->size() << std::endl;
    for (const auto &shape : *this) {
        Vector<3> position = shape.getPosition();
        Matrix<3, 3> orientation = shape.getOrientation();
        out << position[0] << " " << position[1] << " " << position[2];
        out << "        ";
        out << orientation(0, 0) << " " << orientation(0, 1) << " " << orientation(0, 2) << " ";
        out << orientation(1, 0) << " " << orientation(1, 1) << " " << orientation(1, 2) << " ";
        out << orientation(2, 0) << " " << orientation(2, 1) << " " << orientation(2, 2);
        out << std::endl;
    }
}

std::map<std::string, std::string> Packing::restore(std::istream &in, const Interaction &interaction) {
    // Read aux info map
    std::size_t auxInfoSize{};
    in >> auxInfoSize;
    ValidateMsg(in, "Broken packing file: aux info size");
    std::map<std::string, std::string> auxInfo;
    for (std::size_t i{}; i < auxInfoSize; i++) {
        std::string key;
        std::string value;
        in >> key >> std::ws;
        std::getline(in, value);
        ValidateMsg(in, "Broken packing file: aux info entry " + std::to_string(i));
        auxInfo[key] = value;
    }

    // Read box dimensions
    TriclinicBox box_(Packing::restoreDimensions(in));
    Validate(box_.getVolume() != 0);

    // Read particles
    std::size_t size{};
    in >> size;
    ValidateMsg(in, "Broken packing file: size");

    std::vector<Shape> shapes_;
    shapes_.reserve(size + this->moveThreads);  // temp shapes at the back
    for (std::size_t i{}; i < size; i++) {
        Vector<3> position;
        Matrix<3, 3> orientation;
        in >> position[0] >> position[1] >> position[2];
        in >> orientation(0, 0) >> orientation(0, 1) >> orientation(0, 2);
        in >> orientation(1, 0) >> orientation(1, 1) >> orientation(1, 2);
        in >> orientation(2, 0) >> orientation(2, 1) >> orientation(2, 2);
        ValidateMsg(in, "Broken packing file: shape " + std::to_string(shapes_.size()) + "/"
                        + std::to_string(size));
        shapes_.emplace_back(position, orientation);
    }

    // Load data into class
    this->box = box_;
    shapes_.resize(shapes_.size() + this->moveThreads);     // add temp shapes
    this->shapes = shapes_;
    this->bc->setBox(this->box);
    this->setupForInteraction(interaction);

    return auxInfo;
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
    bytes += get_vector_memory_usage(this->shapes);
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
            const auto &pos = this->shapes[i].getPosition();
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
        auto pos = this->shapes[particleIdx].getPosition() + this->interactionCentres[centreIdx];
        this->absoluteInteractionCentres[centreIdx] = pos + this->bc->getCorrection(pos);
    }
}

Matrix<3, 3> Packing::restoreDimensions(std::istream &in) {
    std::string line;
    std::getline(in, line);
    ValidateMsg(in, "Broken packing file: dimensions");

    // We support both new and old format: in the old box was cuboidal, so only 3 numbers were stored. Now we store
    // a whole 9-element box matrix. The format can be recognized by the number of string in the line.
    double tokensOld[3];
    std::istringstream dimensionsStream(line);
    dimensionsStream >> tokensOld[0] >> tokensOld[1] >> tokensOld[2] >> std::ws;
    // in.fail() is error apart from eof, which may happen
    ValidateMsg(!dimensionsStream.fail(), "Broken packing file: dimensions");

    Matrix<3, 3> dimensions;
    if (dimensionsStream.eof()) {     // If eof, dimensions were saved in the old format: L_x, L_y, L_z
        dimensions(0, 0) = tokensOld[0];
        dimensions(1, 1) = tokensOld[1];
        dimensions(2, 2) = tokensOld[2];
    } else {            // Otherwise, new format - box matrix
        dimensions(0, 0) = tokensOld[0];
        dimensions(0, 1) = tokensOld[1];
        dimensions(0, 2) = tokensOld[2];
        dimensionsStream >> dimensions(1, 0) >> dimensions(1, 1) >> dimensions(1, 2);
        dimensionsStream >> dimensions(2, 0) >> dimensions(2, 1) >> dimensions(2, 2);
        ValidateMsg(!dimensionsStream.fail(), "Broken packing file: dimensions");
    }

    return dimensions;
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

