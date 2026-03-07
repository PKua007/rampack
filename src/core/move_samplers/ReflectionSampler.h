//
// Created by ciesla on 1/1/25.
//

#ifndef RAMPACK_REFLECTIONSAMPLER_H
#define RAMPACK_REFLECTIONSAMPLER_H

#include <string>
#include <variant>

#include "core/MoveSampler.h"
#include "core/geometry/GeneralShapeAxis.h"

/**
 * @brief MoveSampler performing the reflection relative to a given plane described by its normal vector.
 * @details <p> Particles are sampled at random. Reflection is realized as a rotation created from two reflections:
 * first through shape's mirror symmetry plane (leaving the shape unchanged) and then through the specified one.
 * Both are defined though the normal vectors of the reflection plane. The latter can be a fixed reflection in lab
 * coordinated, or a relative one in shape coordinates.
 *
 * <p> Internally, it consists of a single move named `reflection`. The group name is:
 * - **for a lab axis**: `"reflection([x],[y],[z])"`, where `[x]`, `[y]`, and `[z]` are full-precision double
 * coordinates (formatted as `%.17g`)
 * - **for a named shape axis**: `"reflection([shape axis])"`, where `[shape axis]` is `primary`, `secondary`, or
 * `auxiliary`
 * - **for a shape-local vector axis**: `"reflection(shape,[x],[y],[z])"`
 */
class ReflectionSampler : public MoveSampler {
private:
    using Axis = std::variant<Vector<3>, GeneralShapeAxis>;

    const Axis reflectionAxis;
    const GeneralShapeAxis reflectionSymmetryAxis;
    const std::size_t reflectEvery{};

    Vector<3> reflectionAxisForCurrentGeometry;
    Vector<3> symmetryPlaneAxisForCurrentGeometry;
    Vector<3> geometricOrigin;
    bool isGeometricOriginZero{};

    [[nodiscard]] Vector<3> prepareReflectionAxis(const Shape &shape) const;
    [[nodiscard]] Vector<3> prepareSymmetryPlaneAxis(const Shape &shape) const;
    [[nodiscard]] std::string getReflectionAxisNameSuffix() const;
    [[nodiscard]] static Matrix<3, 3> getRotationMatrixPretendingToBeReflection(const Vector<3> &reflectionAxis,
                                                                                const Vector<3> &symmetryPlaneAxis);

public:
    /**
     * @brief Constructs the sampler with a shape reflection axis @a reflectionAxis, a shape symmetry axis
     * @a reflectionSymmetryAxis, and reflection period @a reflectEvery.
     */
    ReflectionSampler(const GeneralShapeAxis &reflectionAxis, const GeneralShapeAxis &reflectionSymmetryAxis,
                      std::size_t reflectEvery);

    /**
     * @brief Constructs the sampler with a lab reflection axis @a reflectionAxis, a shape symmetry axis
     * @a reflectionSymmetryAxis, and reflection period @a reflectEvery.
     */
    ReflectionSampler(const Vector<3> &reflectionAxis, const GeneralShapeAxis &reflectionSymmetryAxis,
                      std::size_t reflectEvery);

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override;
    MoveData sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs,
                        std::mt19937 &mt) override;
    bool increaseStepSize() override { return false; }
    bool decreaseStepSize() override { return false; }
    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override {
        return {{"reflection", 0}};
    }
    void setStepSize(const std::string &, double) override { }
    void setupForShapeTraits(const ShapeTraits &shapeTraits) override;
};

#endif //RAMPACK_REFLECTIONSAMPLER_H
