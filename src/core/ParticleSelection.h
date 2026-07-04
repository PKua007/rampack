//
// Created by Codex on 14/06/2026.
//

#ifndef RAMPACK_PARTICLESELECTION_H
#define RAMPACK_PARTICLESELECTION_H

#include <cstddef>
#include <vector>


/**
 * @brief A class specifying particles eligible for a move.
 * @details Selection is defined by mode and optional particle indices. It has to be prepared for the current packing
 * size before active particle indices and membership queries are used.
 */
class ParticleSelection {
public:
    /**
     * @brief Enum class representing particle selection mode.
     */
    enum class Mode {
        /** @brief All particles are selected. */
        ALL,
        /** @brief Only specified particles are selected. */
        WHITELIST,
        /** @brief All particles except specified ones are selected. */
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
    /**
     * @brief Constructs selection containing all particles.
     */
    ParticleSelection() = default;

    /**
     * @brief Constructs selection containing only @a indices.
     * @details Indices are sorted and duplicates are removed.
     */
    [[nodiscard]] static ParticleSelection whitelist(std::vector<std::size_t> indices);

    /**
     * @brief Constructs selection containing all particles except @a indices.
     * @details Indices are sorted and duplicates are removed.
     */
    [[nodiscard]] static ParticleSelection blacklist(std::vector<std::size_t> indices);

    /**
     * @brief Prepares active particle indices and lookup mask for @a numParticles particles.
     * @details Out-of-range specified indices are ignored. For Mode::ALL, active particles are all indices in
     * [0, @a numParticles). For Mode::WHITELIST and Mode::BLACKLIST, active particles are obtained by applying the
     * specified indices to this range.
     */
    void prepare(std::size_t numParticles_);

    /**
     * @brief Returns selection mode.
     */
    [[nodiscard]] Mode getMode() const { return this->mode; }

    /**
     * @brief Returns normalized indices specified for whitelist or blacklist.
     */
    [[nodiscard]] const std::vector<std::size_t> &getSpecifiedParticleIndices() const {
        return this->specifiedParticleIndices;
    }

    /**
     * @brief Returns indices of particles active after the last prepare() call.
     */
    [[nodiscard]] const std::vector<std::size_t> &getActiveParticleIndices() const {
        return this->activeParticleIndices;
    }

    /**
     * @brief Returns @a true if particle with @a particleIdx is active after the last prepare() call.
     */
    [[nodiscard]] bool isParticleActive(std::size_t particleIdx) const;

    /**
     * @brief Returns number of particles active after the last prepare() call.
     */
    [[nodiscard]] std::size_t getNumActiveParticles() const;
};


#endif //RAMPACK_PARTICLESELECTION_H
