//
// Created by Piotr Kubala on 28/12/2022.
//

#ifndef RAMPACK_TRUNCATEDPLAYER_H
#define RAMPACK_TRUNCATEDPLAYER_H

#include <memory>

#include "core/SimulationPlayer.h"


/**
 * @brief Adapted for any SimulationPlayer, which truncated the trajectory on a given number of total cycles.
 */
class TruncatedPlayer : public SimulationPlayer {
private:
    std::unique_ptr<SimulationPlayer> player;
    std::size_t totalCycles{};

public:
    /**
     * @brief Creates the player.
     * @param player player to truncate.
     * @param totalCycles number of cycles to truncate on.
     */
    TruncatedPlayer(std::unique_ptr<SimulationPlayer> player, std::size_t totalCycles);

    void reset() override { this->player->reset(); }
    [[nodiscard]] std::size_t getCurrentSnapshotCycles() const override { return this->player->getCurrentSnapshotCycles(); }
    [[nodiscard]] std::size_t getCycleStep() const override { return this->player->getCycleStep(); }
    [[nodiscard]] std::size_t getNumMolecules() const override { return this->player->getNumMolecules(); }
    void close() override { this->player->close(); }

    [[nodiscard]] bool hasNext() const override;
    void nextSnapshot(Packing &packing, const Interaction &interaction, const ShapeDataManager &dataManager) override;
    void lastSnapshot(Packing &packing, const Interaction &interaction, const ShapeDataManager &dataManager) override;
    void jumpToSnapshot(Packing &packing, const Interaction &interaction, const ShapeDataManager &dataManager,
                        std::size_t cycleNumber) override;
    [[nodiscard]] std::size_t getTotalCycles() const override;
};


#endif //RAMPACK_TRUNCATEDPLAYER_H
