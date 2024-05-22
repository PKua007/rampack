//
// Created by Piotr Kubala on 22/05/2024.
//

#include "TransformingPlayer.h"
#include "core/lattice/LatticeTraits.h"
#include "utils/Exceptions.h"


TransformingPlayer::TransformingPlayer(std::unique_ptr<SimulationPlayer> player,
                                       std::vector<std::shared_ptr<LatticeTransformer>> transformers)
        : player{std::move(player)}, transformers{std::move(transformers)}
{
    Expects(this->player != nullptr);
    this->player->reset();
}

void TransformingPlayer::transformPacking(Packing &packing, const ShapeTraits &traits) const {
    auto lattice = LatticeTraits::latticeFromPacking(packing);
    for (const auto &transformer : this->transformers)
        transformer->transform(lattice, traits);
    packing.reset(lattice.generateMolecules(), lattice.getLatticeBox(), traits.getInteraction(),
                  traits.getDataManager());
}

void TransformingPlayer::nextSnapshot(Packing &packing, const ShapeTraits &traits) {
    this->player->nextSnapshot(packing, traits);
    this->transformPacking(packing, traits);
}

void TransformingPlayer::lastSnapshot(Packing &packing, const ShapeTraits &traits) {
    this->player->lastSnapshot(packing, traits);
    this->transformPacking(packing, traits);
}

void TransformingPlayer::jumpToSnapshot(Packing &packing, const ShapeTraits &traits, std::size_t cycleNumber) {
    this->player->jumpToSnapshot(packing, traits, cycleNumber);
    this->transformPacking(packing, traits);
}
