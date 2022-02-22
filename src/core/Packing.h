//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_PACKING_H
#define RAMPACK_PACKING_H

#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include <map>

#include "Shape.h"
#include "BoundaryConditions.h"
#include "Interaction.h"
#include "ShapePrinter.h"
#include "NeighbourGrid.h"
#include "ActiveDomain.h"
#include "utils/OMPMacros.h"
#include "TriclinicBox.h"

/**
 * @brief A class representing the packing of molecules, eligible for Monte Carlo perturbations.
 * @details The class contains the boundary conditions and neighbour grid acceleration structure, however neither
 * ShapeTraits nor Interaction are remembered, so they can be easily changed on the fly. Packing remembers both
 * molecule centers and interaction center positions for efficient computations and is tailored for multi-threaded
 * operations - the number of OpenMP thread is recognized in single-molecule methods, which as a result can be
 * performed concurrently. Volume moves have a built-in parallelization.
 */
class Packing {
private:
    // shapes, interactionCentres and absoluteInteractionCentres contain additional slots at the end for temporary data
    // for all threads

    // Shapes in the packing - mass centers and orientations
    std::vector<Shape> shapes;
    // Positions of interaction centers with respect to mass centers (coherent with particle orientations)
    std::vector<Vector<3>> interactionCentres;
    // Absolute positions of interaction centers (shapes[i].getPosition() + interactionCentres[i] + bc correction)
    std::vector<Vector<3>> absoluteInteractionCentres;

    TriclinicBox box;
    std::unique_ptr<BoundaryConditions> bc;
    std::optional<NeighbourGrid> neighbourGrid;
    std::optional<NeighbourGrid> tempNeighbourGrid;     // temp ng is used for swapping in volume moves
    double interactionRange{};
    std::size_t numInteractionCentres{};

    std::size_t moveThreads{};
    std::size_t scalingThreads{};

    std::vector<std::size_t> lastAlteredParticleIdx{};
    TriclinicBox lastBox;

    std::size_t neighbourGridRebuilds{};
    std::size_t neighbourGridResizes{};
    double neighbourGridRebuildMicroseconds{};

    void rebuildNeighbourGrid();
    void removeInteractionCentresFromNeighbourGrid(std::size_t particleIdx);
    void addInteractionCentresToNeighbourGrid(std::size_t particleIdx);
    void addInteractionCentresToNeighbourGrid();
    
    void recalculateAbsoluteInteractionCentres();
    void recalculateAbsoluteInteractionCentres(std::size_t particleIdx);

    void prepareTempInteractionCentres(std::size_t particleIdx);
    void rotateTempInteractionCentres(const Matrix<3, 3> &rotation);
    void acceptTempInteractionCentres();


    // In all the methods below, tempParticleIdx means where the position is stored - may be equal to
    // originalParticleIdx or be the last index (temp shape). originalParticleIdx is the actual id of the particle, but
    // the position under it may be not representative at the moment - for example in the process of performing the move

    // "Main hub" for checking a single particle (all cases - with or without neighbour grid, one or many interaction
    // centres
    [[nodiscard]] bool isParticleOverlappingAnything(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                                     const Interaction &interaction) const;
    // Helper method for the overlap check without neighbour grid - exhaustive checks for all interaction centers
    [[nodiscard]] bool overlapBetweenParticlesWithoutNG(std::size_t tempParticleIdx, std::size_t anotherParticleIdx,
                                                        const Interaction &interaction) const;
    // Helper method for a single interaction center with neighbour grid
    [[nodiscard]] bool isInteractionCentreOverlappingAnythingWithNG(std::size_t originalParticleIdx,
                                                                    std::size_t tempParticleIdx, std::size_t centre,
                                                                    const Interaction &interaction) const;
    // Helper method for a single NG cell when checking all particles
    [[nodiscard]] bool areAnyParticlesOverlappingNGCellHelper(const std::array<std::size_t, 3> &coord,
                                                              const Interaction &interaction) const;

    // Analogous helper methods as for overlaps but for energy
    [[nodiscard]] double calculateParticleEnergy(std::size_t originalParticleIdx, std::size_t tempParticleIdx,
                                                 const Interaction &interaction) const;
    [[nodiscard]] double calculateEnergyBetweenParticlesWithoutNG(std::size_t tempParticleIdx,
                                                                  std::size_t anotherParticleIdx,
                                                                  const Interaction &interaction) const;
    [[nodiscard]] double calculateInteractionCentreEnergyWithNG(std::size_t originalParticleIdx,
                                                                std::size_t tempParticleIdx, size_t centre,
                                                                const Interaction &interaction) const;
    [[nodiscard]] double getTotalEnergyNGCellHelper(const std::array<std::size_t, 3> &coord,
                                                    const Interaction &interaction) const;

    using iterator = decltype(shapes)::iterator;

    [[nodiscard]] iterator begin() { return this->shapes.begin(); }
    [[nodiscard]] iterator end() { return this->shapes.end() - this->moveThreads; }

    static Matrix<3, 3> restoreDimensions(std::istream &in);

public:
    using const_iterator = decltype(shapes)::const_iterator;

    /**
     * @brief Creates an empty packing. Packing::restore method can then be used to load shapes.
     * @param bc boundary conditions to use
     * @param moveThreads number of threads used for molecule moves. If 0, all OpenMP threads will be used
     * @param scalingThreads number of threads used for volume moves. If 0, all OpenMP threads will be used
     */
    explicit Packing(std::unique_ptr<BoundaryConditions> bc, std::size_t moveThreads = 0,
                     std::size_t scalingThreads = 0);
    /**
     * @brief Creates a packing from shape vector (TriclinicBox version).
     * @param box triclinic box containing the particles
     * @param shapes shapes in the packing
     * @param bc boundary conditions to use
     * @param interaction interaction between molecules in the packing
     * @param moveThreads number of threads used for molecule moves. If 0, all OpenMP threads will be used
     * @param scalingThreads number of threads used for volume moves. If 0, all OpenMP threads will be used
     */
    Packing(TriclinicBox box, std::vector<Shape> shapes, std::unique_ptr<BoundaryConditions> bc,
            const Interaction &interaction, std::size_t moveThreads = 0, std::size_t scalingThreads = 0);

    /**
     * @brief Creates a packing from shape vector (dimensions array version).
     * @param dimensions dimensions of the packing
     * @param shapes shapes in the packing
     * @param bc boundary conditions to use
     * @param interaction interaction between molecules in the packing
     * @param moveThreads number of threads used for molecule moves. If 0, all OpenMP threads will be used
     * @param scalingThreads number of threads used for volume moves. If 0, all OpenMP threads will be used
     */
    Packing(const std::array<double, 3> &dimensions, std::vector<Shape> shapes, std::unique_ptr<BoundaryConditions> bc,
            const Interaction &interaction, std::size_t moveThreads = 0, std::size_t scalingThreads = 0)
            : Packing(TriclinicBox(dimensions), std::move(shapes), std::move(bc), interaction, moveThreads,
                      scalingThreads)
    { }

    /**
     * @brief Return the number of shapes in the packing.
     */
    [[nodiscard]] std::size_t size() const { return this->shapes.size() - this->moveThreads; }

    /**
     * @brief Returns @a true, if packing is empty, false otherwise.
     */
    [[nodiscard]] bool empty() const { return this->shapes.size() == this->moveThreads; }

    /**
     * @brief Returns the begin iterator over the shapes in the packing.
     */
    [[nodiscard]] const_iterator begin() const { return this->shapes.begin(); }

    /**
     * @brief Returns the end iterator over the shapes in the packing.
     */
    [[nodiscard]] const_iterator end() const { return this->shapes.end() - this->moveThreads; }

    /**
     * @brief Read-only access to @a i -th shape
     */
    [[nodiscard]] const Shape &operator[](std::size_t i) const;

    /**
     * @brief Returns the first shape in the packing.
     */
    [[nodiscard]] const Shape &front() const;

    /**
     * @brief Returns the last shape in the packing.
     */
    [[nodiscard]] const Shape &back() const;

    [[nodiscard]] const TriclinicBox &getBox() const { return this->box; }

    /**
     * @brief Returns the number of molecule move threads passed in the constructor.
     */
    [[nodiscard]] std::size_t getMoveThreads() const { return this->moveThreads; }

    /**
     * @brief Returns the number of volume move threads passed in the constructor.
     */
    [[nodiscard]] std::size_t getScalingThreads() const { return this->scalingThreads; }

    /**
     * @brief Returns the number of neighbour grid cell in each direction.
     */
    [[nodiscard]] std::array<std::size_t, 3> getNeighbourGridCellDivisions() const {
        return this->neighbourGrid->getCellDivisions();
    }

    [[nodiscard]] double getVolume() const;

    /**
     * @brief Calculates the packing fraction of the packing assuming that a single molecule has @a shapeVolume volume.
     */
    [[nodiscard]] double getPackingFraction(double shapeVolume) const;

    /**
     * @brief Return the number density, so the number of particles divided by the volume of the packing.
     */
    [[nodiscard]] double getNumberDensity() const;

    /**
     * @brief Returns energy fluctuations (variance) per molecule computed for @a interaction.
     */
    [[nodiscard]] double getParticleEnergyFluctuations(const Interaction &interaction) const;

    /**
     * @brief Returns the soft potential total energy of the packing for @a interaction.
     */
    [[nodiscard]] double getTotalEnergy(const Interaction &interaction) const;
    [[nodiscard]] bool areAnyParticlesOverlapping(const Interaction &interaction) const;

    /**
     * @brief Tries a translation on a particle of index @a particleIdx by a vector @a translation and returns the
     * energy difference (overlap is reported as infinite energy change).
     * @details The translation is not applied directly to a particle, but stored in a thread-specific auxiliary
     * memory. To apply the translation (for example after checking the Metropolis criterion),
     * Packing::acceptTranslation has to be used. If @a boundaries ActiveDomain is passed, the moves pushing the
     * molecule outside of the boundary return an infinite energy.
     */
    double tryTranslation(std::size_t particleIdx, Vector<3> translation, const Interaction &interaction,
                          std::optional<ActiveDomain> boundaries = std::nullopt);

    /**
     * @brief Tries a rotation on a particle of index @a particleIdx by a vector @a translation and returns the
     * energy difference (overlap is reported as infinite energy change).
     * @details The rotation is not applied directly to a particle, but stored in a thread-specific auxiliary
     * memory. To apply the translation (for example after checking the Metropolis criterion),
     * Packing::acceptRotation has to be used.
     */
    double tryRotation(std::size_t particleIdx, const Matrix<3, 3> &rotation, const Interaction &interaction);

    /**
     * @brief Tries both a translation @a translation and a rotation @a rotation on a particle of index @a particleIdx
     * and returns the energy difference (overlap is reported as infinite energy change).
     * @details The transformations are not applied directly to a particle, but stored in a thread-specific auxiliary
     * memory. To apply the move (for example after checking the Metropolis criterion), Packing::acceptMove has to be
     * used. If @a boundaries ActiveDomain is passed, the moves pushing the molecule outside of the boundary return an
     * infinite energy.
     */
    double tryMove(std::size_t particleIdx, const Vector<3> &translation, const Matrix<3, 3> &rotation,
                   const Interaction &interaction, std::optional<ActiveDomain> boundaries = std::nullopt);

    /**
     * @brief Applies the scaling to the packing and returns the energy difference (overlap is reported as infinite
     * energy change).
     * @details Contrary to molecule moves, scaling is directly applied to the packing. If the move needs to be
     * reverted, Packing::revertScaling method can be used. The method calculates the energy using multiple threads.
     * @param scaleFactor the factors by which corresponding side lengths should be scaled
     * @param interaction interaction to compute the energy
     * @return energy difference between the final and initial state
     */
    double tryScaling(const std::array<double, 3> &scaleFactor, const Interaction &interaction);

    double tryScaling(const TriclinicBox& newBox, const Interaction &interaction);

    /**
     * @brief Similar as Packing::tryScaling(const std::array<double, 3> &, const Interaction &), but with all sides
     * scale by the same factor @a scaleFactor
     */
    double tryScaling(double scaleFactor, const Interaction &interaction) {
        return this->tryScaling({scaleFactor, scaleFactor, scaleFactor}, interaction);
    }

    /**
     * @brief Accepts the translation previously done by Packing::tryTranslation.
     */
    void acceptTranslation();

    /**
     * @brief Accepts the rotation previously done by Packing::tryRotation.
     */
    void acceptRotation();

    /**
     * @brief Accepts the move previously done by Packing::tryMove.
     */
    void acceptMove();

    /**
     * @brief Reverts the scaling previously done by
     * Packing::tryScaling(const std::array<double, 3> &, const Interaction &) or
     * Packing::tryScaling(double, const Interaction &)
     */
    void revertScaling();

    /**
     * @brief Reinitialized the packing for a new interaction @a interaction.
     * @details Old molecule positions and rotations are used, but interaction centers are recalculated and neighbour
     * grid is rebuilt.
     */
    void setupForInteraction(const Interaction &interaction);

    /**
     * @brief Resets all counters (neighbour grid rebuilds, etc.).
     */
    void resetCounters();

    /**
     * @brief Represents a packing as a Wolfram Mathematica code.
     * @param out the output stream to store a packing
     * @param printer the object responsible for representing a single shape
     */
    void toWolfram(std::ostream &out, const ShapePrinter &printer) const;

    /**
     * @brief Stores a packing in an internal representation form.
     * @param out the output stream to store a packing
     * @param auxInfo auxiliary key, value map which can store arbitrary metadata
     */
    void store(std::ostream &out, const std::map<std::string, std::string> &auxInfo) const;

    /**
     * @brief Clears the current packing (if not empty) and loads it from an internal representation format from @a in
     * input stream.
     * @param in the input stream to load a packing from
     * @param interaction the interaction between the molecules used to setup the packing
     * @return an auxiliary key, value map which was stored together with a packing
     */
    std::map<std::string, std::string> restore(std::istream &in, const Interaction &interaction);

    /**
     * @brief Returns the number of neighbour grid complete rebuilds since the last reset.
     */
    [[nodiscard]] std::size_t getNeighbourGridRebuilds() const { return this->neighbourGridRebuilds; }

    /**
     * @brief Returns the number of neighbour grid resizes (some required full rebuilds, some not) since the last reset.
     */
    [[nodiscard]] std::size_t getNeighbourGridResizes() const { return this->neighbourGridResizes; }

    /**
     * @brief Returns the total time in microseconds consumed for neighbour grid rebuilds since the last reset.
     */
    [[nodiscard]] double getNeighbourGridRebuildMicroseconds() const { return this->neighbourGridRebuildMicroseconds; }

    /**
     * @brief Returns an average number of neighbour per particles according to neighbour grid.
     */
    [[nodiscard]] double getAverageNumberOfNeighbours() const;

    /**
     * @brief Returns estimated memory usage of the packing in bytes (excluding the neighbour grid).
     */
    [[nodiscard]] std::size_t getShapesMemoryUsage() const;

    /**
     * @brief Returns estimated memory usage of the neighbour grid used by the packing.
     */
    [[nodiscard]] std::size_t getNeighbourGridMemoryUsage() const;

    /**
     * @brief Auxiliary stream insertion operator outputing textual representation of the packing (used for unit testing
     * and debugging purposes).
     */
    friend std::ostream &operator<<(std::ostream &out, const Packing &packing);
};


#endif //RAMPACK_PACKING_H
