//
// Created by Piotr Kubala on 28/12/2022.
//

#include "TruncatedPlayer.h"
#include "utils/Assertions.h"


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

void TruncatedPlayer::nextSnapshot(Packing &packing, const Interaction &interaction) {
    if (this->getCurrentSnapshotCycles() >= this->totalCycles)
        throw std::runtime_error("TruncatedPlayer::nextSnapshot: the end already reached");

    this->player->nextSnapshot(packing, interaction);
}

void TruncatedPlayer::lastSnapshot(Packing &packing, const Interaction &interaction) {
    this->player->jumpToSnapshot(packing, interaction, this->totalCycles);
}

void TruncatedPlayer::jumpToSnapshot(Packing &packing, const Interaction &interaction, std::size_t cycleNumber) {
    if (cycleNumber > this->totalCycles)
        throw std::runtime_error("TruncatedPlayer::jumpToSnapshot: jumping above the limit");

    this->player->jumpToSnapshot(packing, interaction, cycleNumber);
}

std::size_t TruncatedPlayer::getTotalCycles() const {
    return this->totalCycles;
}
