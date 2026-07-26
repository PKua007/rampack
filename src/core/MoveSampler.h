//
// Created by pkua on 18.03.2022.
//

#ifndef RAMPACK_MOVESAMPLER_H
#define RAMPACK_MOVESAMPLER_H

#include <random>
#include <utility>
#include <vector>

#include "geometry/Matrix.h"
#include "geometry/Vector.h"
#include "core/ParticleSelectable.h"
#include "core/ShapeTraits.h"
#include "core/Packing.h"

/**
 * @brief A class sampling molecule moves in its implementation-dependent way.
 * @details It can sample particle indices in any order and perform any combination of translational and rotational
 * moves (either of them or both). MoveSampler has its main (group) name returned by getName(), however it can consist
 * of more than one move types (for example rototranslation moves consist of rotations and translation). Each of inner
 * moves has its own step size. Step sizes can be increased, decreased, queried and set using move name. The inherited
 * particle selection restricts which particles this sampler can perturb.
 */
class MoveSampler : public ParticleSelectable {
public:
    /**
     * @brief Enum class representing move type.
     */
    enum class MoveType {
        /** @brief Translational move (no rotation). */
        TRANSLATION,
        /** @brief Rotational move (no translation). */
        ROTATION,
        /** @brief Translation and rotation at the same time. */
        ROTOTRANSLATION
    };

    /**
     * @brief Helper struct gathering all information about a single sampled move.
     */
    struct MoveData {
        /** @brief Type of the move. */
        MoveType moveType = MoveType::ROTOTRANSLATION;
        /** @brief Index of the molecule to be perturbed */
        std::size_t particleIdx{};
        /** @brief Translation to be performed (0 if MoveData::moveType is MoveType::ROTATION). */
        Vector<3> translation{};
        /** @brief Rotation to be performed (0 if MoveData::moveType is MoveType::TRANSLATION). */
        Matrix<3, 3> rotation{};
    };

    /**
     * @brief Returns main (group) name of the MoveSampler.
     * @brief Names of inner constituent moves can be obtained as a part of getStepSizes() return value.
     */
    [[nodiscard]] virtual std::string getName() const = 0;

    /**
     * @brief Samples a single move according to current step sizes.
     * @param packing Packing on which the move should be performed (notice it is only used to gather information,
     * the actual move is not applied to it)
     * @param particleIdxs non-empty indices of particles eligible for this sampler in the current move batch
     * @param mt Mersene twister engine
     * @return a sampled move
     */
    virtual MoveData sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs,
                                std::mt19937 &mt) = 0;

    /**
     * @brief Returns how many moves this sampler requests for @a numParticles eligible particles.
     * @details @a numParticles is the size of the particle set currently available to this sampler. It may be smaller
     * than the whole packing because of particle selection or domain decomposition. Implementations should return 0 for
     * 0 eligible particles and at least 1 for non-empty eligible sets that are valid for this sampler.
     */
    [[nodiscard]] virtual std::size_t getNumOfRequestedMoves(std::size_t numParticles) const = 0;

    /**
     * @brief Increases all step sizes, if maximum is not reached.
     * @return @a true if at least one step size was increases, @a false if all reached their maximal values.
     */
    virtual bool increaseStepSize() = 0;

    /**
     * @brief Decreased all step sizes, if minimum is not reached.
     * @return @a true if at least one step size was decreased, @a false if all reached their minimal values.
     */
    virtual bool decreaseStepSize() = 0;

    /**
     * @brief Returns a vector of pairs (move name, step size) for all moves.
     */
    [[nodiscard]] virtual std::vector<std::pair<std::string, double>> getStepSizes() const = 0;

    /**
     * @brief Sets the step size using a name, as returned by getStepSizes().
     */
    virtual void setStepSize(const std::string &stepName, double stepSize) = 0;

    virtual void setupForShapeTraits([[maybe_unused]] const ShapeTraits &shapeTraits) {
        // Do nothing by default
    }

};


#endif //RAMPACK_MOVESAMPLER_H
