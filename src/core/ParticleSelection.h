//
// Created by Codex on 14/06/2026.
//

#ifndef RAMPACK_PARTICLESELECTION_H
#define RAMPACK_PARTICLESELECTION_H

#include <cstddef>
#include <vector>


class ParticleSelection {
public:
    enum class Mode {
        ALL,
        WHITELIST,
        BLACKLIST
    };

private:
    Mode mode = Mode::ALL;
    std::vector<std::size_t> specifiedParticleIndices;
    std::vector<std::size_t> activeParticleIndices;
    std::vector<char> activeMask;
    std::size_t numParticles = 0;
    std::size_t numActiveParticles = 0;

    explicit ParticleSelection(Mode mode_, std::vector<std::size_t> particleIndices);
    void prepareAllActiveMask(std::size_t numParticles_);
    void prepareWhitelistActiveMask(std::size_t numParticles_);
    void prepareBlacklistActiveMask(std::size_t numParticles_);

public:
    ParticleSelection() = default;

    [[nodiscard]] static ParticleSelection whitelist(std::vector<std::size_t> indices);
    [[nodiscard]] static ParticleSelection blacklist(std::vector<std::size_t> indices);

    void prepare(std::size_t numParticles_);

    [[nodiscard]] Mode getMode() const { return this->mode; }
    [[nodiscard]] const std::vector<std::size_t> &getSpecifiedParticleIndices() const {
        return this->specifiedParticleIndices;
    }
    [[nodiscard]] const std::vector<std::size_t> &getActiveParticleIndices() const {
        return this->activeParticleIndices;
    }
    [[nodiscard]] bool isParticleActive(std::size_t particleIdx) const;
    [[nodiscard]] std::size_t getNumActiveParticles() const;
};


#endif //RAMPACK_PARTICLESELECTION_H
