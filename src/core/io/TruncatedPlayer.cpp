//
// Created by Piotr Kubala on 28/12/2022.
//

#include "TruncatedPlayer.h"
#include "utils/Exceptions.h"


TruncatedPlayer::TruncatedPlayer(std::unique_ptr<SimulationPlayer> player, std::size_t totalCycles)
        : player{std::move(player)}, totalCycles{totalCycles}
{
    Expects(this->player != nullptr);
    Expects(totalCycles <= this->player->getTotalCycles());
    Expects(totalCycles % this->player->getCycleStep() == 0);
    this->player->reset();
}

bool TruncatedPlayer::hasNext() const {
    if (!this->player->hasNext())
        return false;

    return this->getCurrentSnapshotCycles() < this->totalCycles;
}

void TruncatedPlayer::nextSnapshot(Packing &packing, const Interaction &interaction,
                                   const ShapeDataManager &dataManager)
{
    if (this->getCurrentSnapshotCycles() >= this->totalCycles)
        throw std::runtime_error("TruncatedPlayer::nextSnapshot: the end already reached");

    this->player->nextSnapshot(packing, interaction, dataManager);
}

void TruncatedPlayer::lastSnapshot(Packing &packing, const Interaction &interaction,
                                   const ShapeDataManager &dataManager)
{
    this->player->jumpToSnapshot(packing, interaction, dataManager, this->totalCycles);
}

void TruncatedPlayer::jumpToSnapshot(Packing &packing, const Interaction &interaction,
                                     const ShapeDataManager &dataManager, std::size_t cycleNumber)
{
    if (cycleNumber > this->totalCycles)
        throw std::runtime_error("TruncatedPlayer::jumpToSnapshot: jumping above the limit");

    this->player->jumpToSnapshot(packing, interaction, dataManager, cycleNumber);
}

std::size_t TruncatedPlayer::getTotalCycles() const {
    return this->totalCycles;
}
