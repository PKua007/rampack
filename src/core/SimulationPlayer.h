//
// Created by pkua on 17.11.22.
//

#ifndef RAMPACK_SIMULATIONPLAYER_H
#define RAMPACK_SIMULATIONPLAYER_H

#include "Packing.h"


/**
 * @brief Class replaying recorded trajectory.
 * @details The format and source of the trajectory is implementation-specific.
 */
class SimulationPlayer {
public:
    virtual ~SimulationPlayer() = default;

    /**
     * @brief Returns true if there is next snapshot to move to.
     */
    [[nodiscard]] virtual bool hasNext() const = 0;

    /**
     * @brief Moves to the next snapshot (the first one is an original invocation) and prints it on @a packing.
     */
    virtual void nextSnapshot(Packing &packing, const Interaction &interaction,
                              const ShapeDataManager &dataManager) = 0;

    /**
     * @brief Moves back to the beginning of the trajectory. Calling nextSnapshot() afterwards will then jump to the
     * first one.
     */
    virtual void reset() = 0;

    /**
     * @brief Moves to the last snapshot and prints it on @a packing.
     */
    virtual void lastSnapshot(Packing &packing, const Interaction &interaction,
                              const ShapeDataManager &dataManager) = 0;

    /**
     * @brief Jumps to a given snapshot and prints it on @a packing.
     */
    virtual void jumpToSnapshot(Packing &packing, const Interaction &interaction, const ShapeDataManager &dataManager,
                                std::size_t cycleNumber) = 0;

    /**
     * @brief Returns number of cycles for a current snapshot (which was recently moved to using
     * SimyulationPlayer::nextSnapshot()).
     */
    [[nodiscard]] virtual std::size_t getCurrentSnapshotCycles() const = 0;

    /**
     * @brief Returns total number of performed MC cycles (together with cycle offset)
     */
    [[nodiscard]] virtual std::size_t getTotalCycles() const = 0;

    /**
     * @brief Returns total number of recorded cycles.
     */
    [[nodiscard]] virtual std::size_t getCycleStep() const = 0;

    /**
     * @brief Returns number of molecules in the trajectory.
     */
    [[nodiscard]] virtual std::size_t getNumMolecules() const = 0;

    /**
     * @brief Closes the underlying source on demand and prevents any further operations.
     */
    virtual void close() = 0;
};


#endif //RAMPACK_SIMULATIONPLAYER_H
