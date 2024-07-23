//
// Created by Michal Ciesla on 09.06.2024.
//

#ifndef RAMPACK_ROTATIONAROUNDAXISSAMPLER_H
#define RAMPACK_ROTATIONAROUNDAXISSAMPLER_H

#include "core/MoveSampler.h"

/**
 * @brief MoveSampler performing the rotational move consisted of two rotations.
 * @details Particles are sampled at random. Rotation is performed around a given axis which can be relative to
 * simulation box coordinate system (@global=True) or to particle coordinate system.
 * The rotation angle is sampled uniformly from an interval given by the current step size. Maximal step size is PI.
 * Internally it consists of a single move named @a rotationAroundAxis. The group name is @a rotation.
 */
class RotationAroundAxisSampler : public MoveSampler {
private:
    double rotationStepSize{};
    size_t particleAxisIdx{};
    Vector<3, double> axis{};
    bool global{};

public:
    /**
     * @brief Constructs the sampler with an initial step size @a rotationStepSize.
     */
    explicit RotationAroundAxisSampler(double rotationStepSize, const Vector<3, double>& axis, bool global);

    [[nodiscard]] std::string getName() const override { return "rotationAroundAxis"; }

    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override { return numParticles; }

    MoveData sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs,
                        std::mt19937 &mt) override;
    bool increaseStepSize() override;
    bool decreaseStepSize() override;

    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override;

    void setStepSize(const std::string &stepName, double stepSize) override;
};

#endif //RAMPACK_ROTATIONAROUNDAXISSAMPLER_H
