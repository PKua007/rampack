//
// Created by Codex on 14/06/2026.
//

#include "ParticleSelection.h"

#include <algorithm>
#include <utility>

#include "utils/Exceptions.h"


ParticleSelection::ParticleSelection(const Mode mode_, std::vector<std::size_t> particleIndices)
        : mode{mode_}, specifiedParticleIndices{std::move(particleIndices)}
{
    std::sort(this->specifiedParticleIndices.begin(), this->specifiedParticleIndices.end());
    auto duplicateStart = std::unique(this->specifiedParticleIndices.begin(), this->specifiedParticleIndices.end());
    this->specifiedParticleIndices.erase(duplicateStart, this->specifiedParticleIndices.end());
}

ParticleSelection ParticleSelection::whitelist(std::vector<std::size_t> indices) {
    return ParticleSelection{Mode::WHITELIST, std::move(indices)};
}

ParticleSelection ParticleSelection::blacklist(std::vector<std::size_t> indices) {
    return ParticleSelection{Mode::BLACKLIST, std::move(indices)};
}

void ParticleSelection::prepare(const std::size_t numParticles_) {
    this->numParticles = numParticles_;
    this->activeParticleIndices.clear();

    switch (this->mode) {
        case Mode::ALL:
            this->prepareAllActiveMask(numParticles_);
            break;
        case Mode::WHITELIST:
            this->prepareWhitelistActiveMask(numParticles_);
            break;
        case Mode::BLACKLIST:
            this->prepareBlacklistActiveMask(numParticles_);
            break;
        default:
            AssertThrow("ParticleSelection::Mode");
    }

    this->activeParticleIndices.reserve(numParticles_);
    for (std::size_t particleIdx{}; particleIdx < numParticles_; ++particleIdx) {
        if (this->activeMask[particleIdx])
            this->activeParticleIndices.push_back(particleIdx);
    }

    this->numActiveParticles = this->activeParticleIndices.size();
}

void ParticleSelection::prepareAllActiveMask(const std::size_t numParticles_) {
    this->activeMask.assign(numParticles_, true);
}

void ParticleSelection::prepareWhitelistActiveMask(const std::size_t numParticles_) {
    this->activeMask.assign(numParticles_, false);

    for (const auto particleIdx : this->specifiedParticleIndices) {
        if (particleIdx < numParticles_)
            this->activeMask[particleIdx] = true;
    }
}

void ParticleSelection::prepareBlacklistActiveMask(const std::size_t numParticles_) {
    this->activeMask.assign(numParticles_, true);

    for (const auto particleIdx : this->specifiedParticleIndices) {
        if (particleIdx < numParticles_)
            this->activeMask[particleIdx] = false;
    }
}

bool ParticleSelection::isParticleActive(const std::size_t particleIdx) const {
    Expects(particleIdx < this->numParticles);

    return this->activeMask[particleIdx];
}

std::size_t ParticleSelection::getNumActiveParticles() const {
    return this->numActiveParticles;
}
