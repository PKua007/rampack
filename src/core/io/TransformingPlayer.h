//
// Created by Piotr Kubala on 22/05/2024.
//

#ifndef RAMPACK_TRANSFORMINGPLAYER_H
#define RAMPACK_TRANSFORMINGPLAYER_H

#include "core/SimulationPlayer.h"
#include "core/lattice/LatticeTransformer.h"


/**
 * @brief Decorator SimulationPlayer which applies a set of (irregular) LatticeTransformers to each replayed snapshot.
 */
class TransformingPlayer : public SimulationPlayer {
private:
    std::unique_ptr<SimulationPlayer> player;
    std::vector<std::shared_ptr<LatticeTransformer>> transformers;
    std::size_t numMolecules{};

    void transformPacking(Packing &packing, const ShapeTraits &traits) const;

public:
    /**
     * @brief Creates the object.
     * @param player underlying player
     * @param transformers transformers which are applied to each replayed snapshot. The transformer may change the
     * number of particles, but their number must be constant for each snapshot
     * @param testPacking test packing which is used to determine the number of particles after @a transformers are
     * applied. If @a player has more than zero snapshots, the last snapshot is printed onto @a testPacking before
     * applying the transformers.
     * @param traits
     */
    TransformingPlayer(std::unique_ptr<SimulationPlayer> player,
                       std::vector<std::shared_ptr<LatticeTransformer>> transformers, Packing &testPacking,
                       const ShapeTraits &traits);

    [[nodiscard]] bool hasNext() const override { return this->player->hasNext(); }
    void reset() override { this->player->reset(); }
    [[nodiscard]] std::size_t getCurrentSnapshotCycles() const override { return player->getCurrentSnapshotCycles(); }
    [[nodiscard]] std::size_t getTotalCycles() const override { return this->player->getTotalCycles(); }
    [[nodiscard]] std::size_t getCycleStep() const override { return this->player->getCycleStep(); }

    /**
     * @brief Returns number of molecules after applying the transformers.
     * @details The number is calculates bases on the test packing (see TransformingPlayer::TransformingPlayer).
     */
    [[nodiscard]] std::size_t getNumMolecules() const override { return this->numMolecules; }

    void close() override { this->player->close(); }

    void nextSnapshot(Packing &packing, const ShapeTraits &traits) override;
    void lastSnapshot(Packing &packing, const ShapeTraits &traits) override;
    void jumpToSnapshot(Packing &packing, const ShapeTraits &traits, std::size_t cycleNumber) override;
};


#endif //RAMPACK_TRANSFORMINGPLAYER_H
