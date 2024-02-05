//
// Created by Michal Ciesla on 09.06.2024.
//

#ifndef RAMPACK_AXIALROTATIONSAMPLER_H
#define RAMPACK_AXIALROTATIONSAMPLER_H

#include <variant>

#include "core/MoveSampler.h"


/**
 * @brief MoveSampler performing the rotations around a fixed global or shape axis.
 * @details Particles are sampled at random. Rotation is performed around a fixed, global axis or shape axis (primary,
 * secondary, or auxiliary, see ShapeGeometry). The rotation angle is sampled uniformly from an interval given by the
 * current step size. Maximal step size is &pi;.
 *
 * Internally it consists of a single move named `rotation`. The group name is:
 * - **for a global axis**: `"axis_rotation([x],[y],[z])"`, where `[x]`, `[y]`, and `[z]` are full-precision double
 * coordinates (formatted as `%.17g`)
 * - **for a shape axis**: `"axis_rotation([shape axis])"`, where `[shape axis]` is `primary`, `secondary`, or `auxiliary`
 */
class AxialRotationSampler : public MoveSampler {
public:
    /**
     * @brief Axis variant: Vector<3> - global axis, ShapeGeometry:::Axis - shape axis.
     */
    using Axis = std::variant<Vector<3>, ShapeGeometry::Axis>;

private:
    [[nodiscard]] Vector<3> computeAxis(const Shape &shape) const;

    double rotationStepSize{};
    const ShapeGeometry *geometry = nullptr;
    Axis axis{};

public:
    /**
     * @brief Constructs the sampler with an initial step size @a rotationStepSize and axis @a axis.
     */
    AxialRotationSampler(double rotationStepSize, const Axis &axis);

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override { return numParticles; }
    MoveData sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs,
                        std::mt19937 &mt) override;
    bool increaseStepSize() override;
    bool decreaseStepSize() override;
    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override;
    void setStepSize(const std::string &stepName, double stepSize) override;
    void setup(const Packing &packing, const ShapeTraits &shapeTraits) override;
};

#endif //RAMPACK_AXIALROTATIONSAMPLER_H
