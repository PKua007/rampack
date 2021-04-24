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


Packing::Packing(const std::array<double, 3> &dimensions, std::vector<Shape> shapes,
                 std::unique_ptr<BoundaryConditions> bc, const Interaction &interaction, std::size_t moveThreads,
                 std::size_t scalingThreads)
        : shapes{std::move(shapes)}, dimensions{dimensions}, bc{std::move(bc)},
          interactionRange{interaction.getRangeRadius()}
{
    Expects(std::all_of(dimensions.begin(), dimensions.end(), [](double d) { return d > 0; }));
    Expects(this->interactionRange > 0);
    Expects(!this->shapes.empty());

    this->moveThreads = (moveThreads == 0 ? _OMP_MAXTHREADS : moveThreads);
    this->scalingThreads = (scalingThreads == 0 ? _OMP_MAXTHREADS : scalingThreads);

    this->shapes.resize(this->shapes.size() + this->moveThreads);    // temp shapes at the back
    this->lastAlteredParticleIdx.resize(this->moveThreads, 0);
    this->bc->setLinearSize(this->dimensions);
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

    this->prepareTempInteractionCentres(particleIdx);
    if (interaction.hasHardPart() && this->isParticleOverlappingAnything(particleIdx, tempParticleIdx, interaction))
        return std::numeric_limits<double>::infinity();

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

    this->prepareTempInteractionCentres(particleIdx);
    this->shapes[tempParticleIdx].rotate(rotation);
    if (this->numInteractionCentres != 0)
        this->rotateTempInteractionCentres(rotation);

    if (interaction.hasHardPart() && this->isParticleOverlappingAnything(particleIdx, tempParticleIdx, interaction))
        return std::numeric_limits<double>::infinity();

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

    this->prepareTempInteractionCentres(particleIdx);
    this->shapes[tempParticleIdx].rotate(rotation);
    if (this->numInteractionCentres != 0)
        this->rotateTempInteractionCentres(rotation);

    if (interaction.hasHardPart() && this->isParticleOverlappingAnything(particleIdx, tempParticleIdx, interaction))
        return std::numeric_limits<double>::infinity();

    double initialEnergy = this->calculateParticleEnergy(particleIdx, particleIdx, interaction);
    double finalEnergy = this->calculateParticleEnergy(particleIdx, tempParticleIdx, interaction);
    return finalEnergy - initialEnergy;
}

double Packing::tryScaling(const std::array<double, 3> &scaleFactor, const Interaction &interaction) {
    Expects(std::all_of(scaleFactor.begin(), scaleFactor.end(), [](double d) { return d > 0; }));
    Expects(interaction.getRangeRadius() <= this->interactionRange);
    this->lastDimensions = this->dimensions;

    double initialEnergy = this->getTotalEnergy(interaction);

    std::transform(this->dimensions.begin(), this->dimensions.end(), scaleFactor.begin(), this->dimensions.begin(),
                   std::multiplies<>{});
    this->bc->setLinearSize(this->dimensions);
    for (auto &shape : *this)
        shape.scale(scaleFactor);
    std::swap(this->neighbourGrid, this->tempNeighbourGrid);
    this->rebuildNeighbourGrid();

    if (interaction.hasHardPart() /*&& scaleFactor < 1*/ && this->areAnyParticlesOverlapping(interaction))
        return std::numeric_limits<double>::infinity();

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

    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->add(lastAlteredIdx, this->shapes[lastAlteredIdx].getPosition());
        else
            this->addInteractionCentresToNeighbourGrid(lastAlteredIdx);
    }
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
}

void Packing::addInteractionCentresToNeighbourGrid(std::size_t particleIdx) {
    for (size_t i{}; i < this->numInteractionCentres; i++) {
        std::size_t centreIdx = particleIdx * this->numInteractionCentres + i;
        auto centrePos = this->shapes[particleIdx].getPosition() + this->interactionCentres[centreIdx];
        centrePos += this->bc->getCorrection(centrePos);
        this->neighbourGrid->add(centreIdx, centrePos);
    }
}

void Packing::removeInteractionCentresFromNeighbourGrid(std::size_t particleIdx) {
    for (size_t i{}; i < this->numInteractionCentres; i++) {
        std::size_t centreIdx = particleIdx * this->numInteractionCentres + i;
        auto centrePos = this->shapes[particleIdx].getPosition() + this->interactionCentres[centreIdx];
        centrePos += this->bc->getCorrection(centrePos);
        this->neighbourGrid->remove(centreIdx, centrePos);
    }
}

void Packing::acceptTempInteractionCentres() {
    std::size_t fromOrigin = (this->size() + _OMP_THREAD_ID) * this->numInteractionCentres;
    std::size_t toOrigin = this->lastAlteredParticleIdx[_OMP_THREAD_ID] * this->numInteractionCentres;
    for (std::size_t i{}; i < this->numInteractionCentres; i++)
        this->interactionCentres[toOrigin + i] = this->interactionCentres[fromOrigin + i];
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
    std::array<double, 3> reverseFactor{};
    std::transform(this->lastDimensions.begin(), this->lastDimensions.end(), this->dimensions.begin(),
                   reverseFactor.begin(), std::divides<>{});
    for (auto &shape : this->shapes)
        shape.scale(reverseFactor);

    this->dimensions = this->lastDimensions;
    this->bc->setLinearSize(this->dimensions);
    std::swap(this->neighbourGrid, this->tempNeighbourGrid);
}

bool Packing::isParticleOverlappingAnything(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                            const Interaction &interaction) const
{
    if (this->neighbourGrid.has_value()) {
        if (this->numInteractionCentres == 0) {
            Vector<3> pos = this->shapes[tempParticleIdx].getPosition();
            for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos))
                for (auto j : cell)
                    if (originalParticleIdx != j && this->overlapBetweenParticles(tempParticleIdx, j, interaction))
                        return true;
        } else {
            for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++)
                if (isInteractionCentreOverlappingAnything(originalParticleIdx, tempParticleIdx, centre1, interaction))
                    return true;
        }
    } else {
        for (std::size_t j{}; j < this->size(); j++) {
            if (originalParticleIdx == j)
                continue;
            if (this->overlapBetweenParticles(tempParticleIdx, j, interaction))
                return true;
        }
    }
    return false;
}

bool Packing::areAnyParticlesOverlapping(const Interaction &interaction) const {
    if (this->neighbourGrid.has_value()) {
        std::atomic<bool> overlapFound = false;
        #pragma omp parallel for default(none) shared(overlapFound, interaction) num_threads(this->scalingThreads)
        for (std::size_t i = 0; i < this->size(); i++) {
            if (overlapFound.load(std::memory_order_relaxed))
                continue;
            if (this->isParticleOverlappingAnything(i, i, interaction))
                overlapFound.store(true, std::memory_order_relaxed);
        }

        return overlapFound;
    } else {
        for (std::size_t i{}; i < this->size(); i++)
            for (std::size_t j = i + 1; j < this->size(); j++)
                if (this->overlapBetweenParticles(i, j, interaction))
                    return true;
    }
    return false;
}

bool Packing::overlapBetweenParticles(std::size_t tempParticleIdx, std::size_t anotherParticleIdx,
                                      const Interaction &interaction) const
{
    if (this->numInteractionCentres == 0) {
        if (interaction.overlapBetween(this->shapes[tempParticleIdx].getPosition(),
                                       this->shapes[tempParticleIdx].getOrientation(),
                                       0,
                                       this->shapes[anotherParticleIdx].getPosition(),
                                       this->shapes[anotherParticleIdx].getOrientation(),
                                       0,
                                       *this->bc))
        {
            return true;
        }
    } else {
        for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++) {
            for (std::size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                std::size_t centreIdx1 = tempParticleIdx * this->numInteractionCentres + centre1;
                auto pos1 = this->shapes[tempParticleIdx].getPosition() + this->interactionCentres[centreIdx1];
                const auto &orientation1 = this->shapes[tempParticleIdx].getOrientation();
                std::size_t centreIdx2 = anotherParticleIdx * this->numInteractionCentres + centre2;
                auto pos2 = this->shapes[anotherParticleIdx].getPosition() + this->interactionCentres[centreIdx2];
                const auto &orientation2 = this->shapes[anotherParticleIdx].getOrientation();
                if (interaction.overlapBetween(pos1, orientation1, centre1, pos2, orientation2, centre2, *this->bc))
                    return true;
            }
        }
    }
    return false;
}

bool Packing::isInteractionCentreOverlappingAnything(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                                     std::size_t centre, const Interaction &interaction) const
{
    Expects(this->neighbourGrid.has_value());

    std::size_t centreIdx1 = tempParticleIdx * this->numInteractionCentres + centre;
    auto pos1 = this->shapes[tempParticleIdx].getPosition() + this->interactionCentres[centreIdx1];
    const auto &orientation1 = this->shapes[tempParticleIdx].getOrientation();
    pos1 += this->bc->getCorrection(pos1);
    for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos1)) {
        for (auto centreIdx2 : cell) { // NOLINT(readability-use-anyofallof)
            std::size_t j = centreIdx2 / this->numInteractionCentres;
            if (j == originalParticleIdx)
                continue;
            std::size_t centre2 = centreIdx2 % this->numInteractionCentres;
            auto pos2 = this->shapes[j].getPosition() + this->interactionCentres[centreIdx2];
            const auto &orientation2 = this->shapes[j].getOrientation();
            if (interaction.overlapBetween(pos1, orientation1, centre, pos2, orientation2, centre2, *this->bc))
                return true;
        }
    }

    return false;
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
                for (auto j : cell) {
                    if (originalParticleIdx == j)
                        continue;
                    energy += this->calculateEnergyBetweenParticles(tempParticleIdx, j, interaction);
                }
            }
        } else {
            for (std::size_t centre1{}; centre1 < this->numInteractionCentres; centre1++)
                energy += calculateInteractionCentreEnergy(originalParticleIdx, tempParticleIdx, centre1, interaction);
        }
    } else {
        for (std::size_t j{}; j < this->size(); j++) {
            if (originalParticleIdx == j)
                continue;
            energy += this->calculateEnergyBetweenParticles(tempParticleIdx, j, interaction);
        }
    }
    return energy;
}

double Packing::getTotalEnergy(const Interaction &interaction) const {
    if (!interaction.hasSoftPart())
        return 0;

    double energy{};
    if (this->neighbourGrid.has_value()) {
        #pragma omp parallel for default(none) shared(interaction) reduction(+:energy) num_threads(this->scalingThreads)
        for (std::size_t i = 0; i < this->size(); i++)
            energy += this->calculateParticleEnergy(i, i, interaction)/2;  // Pairs counted twice
    } else {
        for (std::size_t i{}; i < this->size(); i++)
            for (std::size_t j = i + 1; j < this->size(); j++)
                energy += this->calculateEnergyBetweenParticles(i, j, interaction);
    }
    return energy;
}

double Packing::calculateEnergyBetweenParticles(std::size_t tempParticleIdx, std::size_t anotherParticleIdx,
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
            for (size_t centre2{}; centre2 < this->numInteractionCentres; centre2++) {
                std::size_t centreIdx1 = tempParticleIdx * this->numInteractionCentres + centre1;
                auto pos1 = this->shapes[tempParticleIdx].getPosition() + this->interactionCentres[centreIdx1];
                const auto &orientation1 = this->shapes[tempParticleIdx].getOrientation();
                std::size_t centreIdx2 = anotherParticleIdx * this->numInteractionCentres + centre2;
                auto pos2 = this->shapes[anotherParticleIdx].getPosition() + this->interactionCentres[centreIdx2];
                const auto &orientation2 =  this->shapes[anotherParticleIdx].getOrientation();
                energy += interaction.calculateEnergyBetween(pos1, orientation1, centre1, pos2, orientation2, centre2,
                                                             *this->bc);
            }
        }
    }
    return energy;
}

double Packing::calculateInteractionCentreEnergy(size_t originalParticleIdx, std::size_t tempParticleIdx, size_t centre,
                                          const Interaction &interaction) const
{
    Expects(this->neighbourGrid.has_value());

    double energy{};

    std::size_t centreIdx1 = tempParticleIdx * this->numInteractionCentres + centre;
    auto pos1 = this->shapes[tempParticleIdx].getPosition() + this->interactionCentres[centreIdx1];
    pos1 += this->bc->getCorrection(pos1);
    const auto &orientation1 = this->shapes[tempParticleIdx].getOrientation();
    for (const auto &cell : this->neighbourGrid->getNeighbouringCells(pos1)) {
        for (auto centreIdx2 : cell) {
            size_t j = centreIdx2 / this->numInteractionCentres;
            if (j == originalParticleIdx)
                continue;
            size_t centre2 = centreIdx2 % this->numInteractionCentres;
            auto pos2 = this->shapes[j].getPosition() + this->interactionCentres[centreIdx2];
            const auto &orientation2 = this->shapes[j].getOrientation();
            energy += interaction.calculateEnergyBetween(pos1, orientation1, centre, pos2, orientation2, centre2,
                                                         *this->bc);
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
    if (cellSize * 4 > *std::min_element(this->dimensions.begin(), this->dimensions.end())) {
        this->neighbourGrid = std::nullopt;
        return;
    }

    if (!this->neighbourGrid.has_value())
        this->neighbourGrid = NeighbourGrid(this->dimensions, cellSize);
    else
        this->neighbourGridResizes += this->neighbourGrid->resize(this->dimensions, cellSize);

    for (std::size_t i{}; i < this->size(); i++) {
        if (this->numInteractionCentres == 0)
            this->neighbourGrid->add(i, this->shapes[i].getPosition());
        else
            this->addInteractionCentresToNeighbourGrid(i);
    }

    this->neighbourGridRebuilds++;
    auto end = high_resolution_clock::now();
    this->neighbourGridRebuildMicroseconds += duration<double, std::micro>(end - start).count();
}

void Packing::setupForInteraction(const Interaction &interaction) {
    this->interactionRange = interaction.getRangeRadius();
    this->numInteractionCentres = interaction.getInteractionCentres().size();
    this->interactionCentres.clear();
    if (this->numInteractionCentres > 0) {
        // Takes into account temp shapes at the back
        this->interactionCentres.reserve(this->shapes.size() * this->numInteractionCentres);
        auto centres = interaction.getInteractionCentres();
        for (const auto &shape : this->shapes)
            for (const auto &centre : centres)
                this->interactionCentres.emplace_back(shape.getOrientation() * centre);
    }
    this->rebuildNeighbourGrid();
}

double Packing::getVolume() const {
    return std::accumulate(this->dimensions.begin(), this->dimensions.end(), 1., std::multiplies<>{});
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
    out << this->dimensions[0] << " " << this->dimensions[1] << " " << this->dimensions[2] << std::endl;
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
    std::array<double, 3> dimensions_{};
    in >> dimensions_[0] >> dimensions_[1] >> dimensions_[2];
    ValidateMsg(in, "Broken packing file: dimensions");
    Validate(std::all_of(dimensions_.begin(), dimensions_.end(), [](double d) { return d > 0; }));

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
    this->dimensions = dimensions_;
    shapes_.resize(shapes_.size() + this->moveThreads);     // add temp shapes
    this->shapes = shapes_;
    this->bc->setLinearSize(dimensions_);
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
    out << "  dimensions: {";
    out << packing.dimensions[0] << ", " << packing.dimensions[1] << ", " << packing.dimensions[2] << "}," << std::endl;
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

