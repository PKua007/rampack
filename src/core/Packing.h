//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_PACKING_H
#define RAMPACK_PACKING_H

#include <vector>
#include <memory>
#include <optional>

#include "Shape.h"
#include "BoundaryConditions.h"
#include "Interaction.h"
#include "ShapePrinter.h"
#include "NeighbourGrid.h"
#include "ActiveDomain.h"
#include "utils/OMPMacros.h"

class Packing {
private:
    std::vector<Shape> shapes;  // Shapes in the packing - the last shape is a temporary slot
    std::vector<Vector<3>> interactionCentres;  // Interaction centres - last numInteractionCentres are temporary slots
    std::array<double, 3> dimensions{};
    std::unique_ptr<BoundaryConditions> bc;
    std::optional<NeighbourGrid> neighbourGrid;
    std::optional<NeighbourGrid> tempNeighbourGrid;
    double interactionRange{};
    std::size_t numInteractionCentres{};

    std::size_t moveThreads{};
    std::size_t scalingThreads{};

    std::vector<std::size_t> lastAlteredParticleIdx{};
    std::array<double, 3> lastScalingFactor{};

    std::size_t neighbourGridRebuilds{};
    std::size_t neighbourGridResizes{};
    double neighbourGridRebuildMicroseconds{};

    void rebuildNeighbourGrid();
    void removeInteractionCentresFromNeighbourGrid(size_t particleIdx);
    void addInteractionCentresToNeighbourGrid(size_t particleIdx);

    void prepareTempInteractionCentres(std::size_t particleIdx);
    void rotateTempInteractionCentres(const Matrix<3, 3> &rotation);
    void acceptTempInteractionCentres();

    // In all the methods below, tempParticleIdx means where the position is stored - may be equal to
    // originalParticleIdx or be the last index (temp shape). originalParticleIdx is the actual id of the particle, but
    // the position under it may be not representative at the moment - for example in the process of performing the move

    [[nodiscard]] bool areAnyParticlesOverlapping(const Interaction &interaction) const;
    [[nodiscard]] bool isParticleOverlappingAnything(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                                     const Interaction &interaction) const;
    [[nodiscard]] bool overlapBetweenParticles(std::size_t tempParticleIdx, std::size_t anotherParticleIdx,
                                               const Interaction &interaction) const;
    [[nodiscard]] bool isInteractionCentreOverlappingAnything(std::size_t originalParticleIdx,
                                                              std::size_t tempParticleIdx, std::size_t centre,
                                                              const Interaction &interaction) const;

    [[nodiscard]] double calculateParticleEnergy(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                                 const Interaction &interaction) const;
    [[nodiscard]] double calculateEnergyBetweenParticles(std::size_t tempParticleIdx,
                                                         std::size_t anotherParticleIdx,
                                                         const Interaction &interaction) const;
    [[nodiscard]] double calculateInteractionCentreEnergy(std::size_t originalParticleIdx,
                                                          std::size_t tempParticleIdx, size_t centre,
                                                          const Interaction &interaction) const;

    using iterator = decltype(shapes)::iterator;

    [[nodiscard]] iterator begin() { return this->shapes.begin(); }
    [[nodiscard]] iterator end() { return this->shapes.end() - this->moveThreads; }

public:
    using const_iterator = decltype(shapes)::const_iterator;

    explicit Packing(std::unique_ptr<BoundaryConditions> bc, std::size_t moveThreads = 0,
                     std::size_t scalingThreads = 0);
    Packing(const std::array<double, 3> &dimensions, std::vector<Shape> shapes, std::unique_ptr<BoundaryConditions> bc,
            const Interaction &interaction, std::size_t moveThreads = 0, std::size_t scalingThreads = 0);

    [[nodiscard]] std::size_t size() const { return this->shapes.size() - this->moveThreads; }
    [[nodiscard]] bool empty() const { return this->shapes.size() == this->moveThreads; }
    [[nodiscard]] const_iterator begin() const { return this->shapes.begin(); }
    [[nodiscard]] const_iterator end() const { return this->shapes.end() - this->moveThreads; }
    [[nodiscard]] const Shape &operator[](std::size_t i) const;
    [[nodiscard]] const Shape &front() const;
    [[nodiscard]] const Shape &back() const;

    [[nodiscard]] const std::array<double, 3> &getDimensions() const { return this->dimensions; }
    [[nodiscard]] double getMoveThreads() const { return this->moveThreads; }
    [[nodiscard]] double getScalingThreads() const { return this->scalingThreads; }
    [[nodiscard]] std::array<std::size_t, 3> getNeighbourGridCellDivisions() const {
        return this->neighbourGrid->getCellDivisions();
    }
    [[nodiscard]] double getVolume() const;
    [[nodiscard]] double getPackingFraction(double shapeVolume) const;
    [[nodiscard]] double getNumberDensity() const;
    [[nodiscard]] double getParticleEnergyFluctuations(const Interaction &interaction) const;
    [[nodiscard]] double getTotalEnergy(const Interaction &interaction) const;

    double tryTranslation(std::size_t particleIdx, Vector<3> translation, const Interaction &interaction,
                          std::optional<ActiveDomain> boundaries = std::nullopt);
    double tryRotation(std::size_t particleIdx, const Matrix<3, 3> &rotation, const Interaction &interaction);
    double tryMove(std::size_t particleIdx, const Vector<3> &translation, const Matrix<3, 3> &rotation,
                   const Interaction &interaction, std::optional<ActiveDomain> boundaries = std::nullopt);
    double tryScaling(const std::array<double, 3> &scaleFactor, const Interaction &interaction);
    void acceptTranslation();
    void acceptRotation();
    void acceptMove();
    void revertScaling();

    void setupForInteraction(const Interaction &interaction);
    void resetCounters();
    [[nodiscard]] std::size_t getNeighbourGridRebuilds() const { return this->neighbourGridRebuilds; }
    [[nodiscard]] std::size_t getNeighbourGridResizes() const { return this->neighbourGridResizes; }
    [[nodiscard]] double getNeighbourGridRebuildMicroseconds() const { return this->neighbourGridRebuildMicroseconds; }

    void toWolfram(std::ostream &out, const ShapePrinter &printer) const;
    void store(std::ostream &out) const;
    void restore(std::istream &in, const Interaction &interaction);
};


#endif //RAMPACK_PACKING_H
