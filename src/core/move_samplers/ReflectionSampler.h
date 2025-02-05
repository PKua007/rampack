//
// Created by ciesla on 1/1/25.
//

#ifndef RAMPACK_REFLECTIONSAMPLER_H
#define RAMPACK_REFLECTIONSAMPLER_H

#include <variant>

#include "core/MoveSampler.h"

/**
 * @brief ReflectionSampler performing the reflection relative to a given plane described by its normal vector.
 * @details Particles are sampled at random. Reflection is performed in respect to a given plane which can be relative to
 * simulation box coordinate system (@global=True) or to particle coordinate system.
 * Internally it consists of a single move named @a reflection. The group name is @a reflection.
 */
class ReflectionSampler : public MoveSampler{

private:
    std::size_t flipEvery{};
    const ShapeGeometry *geometry = nullptr;
    Vector<3> planeAxis{};

public:
    /**
     * @brief Constructs the class specifying how often to perform a reflection (i.e. how many moves should be requested,
     * calculated by dividing the number of molecules by @a flipEvery).
     */

    explicit ReflectionSampler(std::size_t flipEvery, Vector<3> plane);

    [[nodiscard]] std::string getName() const override { return "reflection"; }

    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override;

    MoveData sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs,
                        std::mt19937 &mt) override;

    bool increaseStepSize() override { return false; }

    bool decreaseStepSize() override { return false; }

    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override { return {{"rotation", 0}};};

    void setStepSize(const std::string &, double) override { }

    void setupForShapeTraits(const ShapeTraits &shapeTraits) override {
        this->geometry = &shapeTraits.getGeometry();
    }
};

#endif //RAMPACK_REFLECTIONSAMPLER_H