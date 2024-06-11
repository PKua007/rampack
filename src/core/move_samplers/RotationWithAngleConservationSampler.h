//
// Created by ciesla on 09.06.2024.
//

#ifndef RAMPACK_ROTATIONWITHANGLECONSERVATIONSAMPLER_H
#define RAMPACK_ROTATIONWITHANGLECONSERVATIONSAMPLER_H

#include "core/MoveSampler.h"

/**
 * @brief MoveSampler performing the rotational move consisted of two rotations.
 * @details Particles are sampled at random. Rotation is performed around a given global axis or around the particle axis, thus the angle between particle axis and given global axis is conserved.
 * The rotation angle is sampled uniformly from an interval given by the current step size. Maximal step size is PI.
 * Internally it consists of a single move named @a rotationWithAngleConservation. The group name is also @a rotation.
 */
class RotationWithAngleConservationSampler : public MoveSampler {
private:
    double rotationStepSize{};
    size_t particleAxisIdx{};
    Vector<3, double> particleAxis{};
    Vector<3, double> globalAxis{};

public:
    /**
     * @brief Constructs the sampler with an initial step size @a rotationStepSize.
     */
    explicit RotationWithAngleConservationSampler(double rotationStepSize, size_t particleAxis, const Vector<3, double>& globalAxis);

    [[nodiscard]] std::string getName() const override { return "rotationWithAngleConservation"; }

    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override { return numParticles; }

    MoveData sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs,
                        std::mt19937 &mt) override;
    bool increaseStepSize() override;
    bool decreaseStepSize() override;

    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override;

    void setStepSize(const std::string &stepName, double stepSize) override;

    void setupForShapeTraits(const ShapeTraits &shapeTraits) override;
};

#endif //RAMPACK_ROTATIONWITHANGLECONSERVATIONSAMPLER_H
