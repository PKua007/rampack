//
// Created by Piotr Kubala on 22/05/2024.
//

#ifndef RAMPACK_TRANSFORMINGPLAYER_H
#define RAMPACK_TRANSFORMINGPLAYER_H

#include "core/SimulationPlayer.h"
#include "core/lattice/LatticeTransformer.h"


class TransformingPlayer : public SimulationPlayer {
private:
    std::unique_ptr<SimulationPlayer> player;
    std::vector<std::shared_ptr<LatticeTransformer>> transformers;

    void transformPacking(Packing &packing, const ShapeTraits &traits) const;

public:
    TransformingPlayer(std::unique_ptr<SimulationPlayer> player,
                       std::vector<std::shared_ptr<LatticeTransformer>> transformers);

    [[nodiscard]] bool hasNext() const override { return this->player->hasNext(); }
    void reset() override { this->player->reset(); }
    [[nodiscard]] std::size_t getCurrentSnapshotCycles() const override { return player->getCurrentSnapshotCycles(); }
    [[nodiscard]] std::size_t getTotalCycles() const override { return this->player->getTotalCycles(); }
    [[nodiscard]] std::size_t getCycleStep() const override { return this->player->getCycleStep(); }
    [[nodiscard]] std::size_t getNumMolecules() const override { return this->player->getNumMolecules(); }
    void close() override { this->player->close(); }

    void nextSnapshot(Packing &packing, const ShapeTraits &traits) override;
    void lastSnapshot(Packing &packing, const ShapeTraits &traits) override;
    void jumpToSnapshot(Packing &packing, const ShapeTraits &traits, std::size_t cycleNumber) override;
};


#endif //RAMPACK_TRANSFORMINGPLAYER_H
