//
// Created by pkua on 21.03.2022.
//

#ifndef RAMPACK_ROTATIONSAMPLER_H
#define RAMPACK_ROTATIONSAMPLER_H

#include "core/MoveSampler.h"

/**
 * @brief MoveSampler performing only the rotational moves.
 * @details Particles are sampled at random. Rotation is performed around a random axis by an angles sampled uniformly
 * from an interval given by the current step size. Maximal step size is PI. Internally it consists of a single move
 * named @a rotation. The group name is also @a rotation.
 */
class RotationSampler : public MoveSampler {
private:
    double rotationStepSize{};

public:
    /**
     * @brief Constructs the sampler with an initial step size @a rotationStepSize.
     */
    explicit RotationSampler(double rotationStepSize);

    [[nodiscard]] std::string getName() const override { return "rotation"; }

    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override { return numParticles; }

    MoveData sampleMove(const Packing &packing, const ShapeTraits &shapeTraits,
                        const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt) override;
    bool increaseStepSize() override;
    bool decreaseStepSize() override;

    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override;

    void setStepSize(const std::string &stepName, double stepSize) override;
};


#endif //RAMPACK_ROTATIONSAMPLER_H
