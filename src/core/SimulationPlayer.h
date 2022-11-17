//
// Created by pkua on 17.11.22.
//

#ifndef RAMPACK_SIMULATIONPLAYER_H
#define RAMPACK_SIMULATIONPLAYER_H

#include "Packing.h"


class SimulationPlayer {
public:
    virtual ~SimulationPlayer() = default;
    [[nodiscard]] virtual bool hasNext() const = 0;
    virtual void nextSnapshot(Packing &packing, const Interaction &interaction) = 0;
    virtual void reset() = 0;
    virtual void lastSnapshot(Packing &packing, const Interaction &interaction) = 0;
    virtual void jumpToSnapshot(Packing &packing, const Interaction &interaction, std::size_t cycleNumber) = 0;
    [[nodiscard]] virtual std::size_t getCurrentSnapshotCycles() const = 0;
    [[nodiscard]] virtual std::size_t getTotalCycles() const = 0;
    [[nodiscard]] virtual std::size_t getCycleStep() const = 0;
    [[nodiscard]] virtual std::size_t getNumMolecules() const = 0;
    virtual void close() = 0;
};


#endif //RAMPACK_SIMULATIONPLAYER_H
