//
// Created by ciesla on 1/1/25.
//

#ifndef RAMPACK_REFLECTIONSAMPLER_H
#define RAMPACK_REFLECTIONSAMPLER_H

#include <variant>

#include "core/MoveSampler.h"
#include "core/geometry/GeneralizedShapeAxis.h"

/**
 * @brief ReflectionSampler performing the reflection relative to a given plane described by its normal vector.
 * @details Particles are sampled at random. Reflection is performed in respect to a given plane which can be relative to
 * simulation box coordinate system (@global=True) or to particle coordinate system.
 * Internally it consists of a single move named @a reflection. The group name is @a reflection.
 */
class ReflectionSampler : public MoveSampler {
public:
    struct AxisOrthogonalToPrimary {};
    using FlipSymmetryAxis = std::variant<ShapeGeometry::Axis, AxisOrthogonalToPrimary>;

private:
    const GeneralizedShapeAxis reflectionAxis;
    const FlipSymmetryAxis flipSymmetryAxis;
    const std::size_t flipEvery{};

    const ShapeGeometry *geometry{};
    Vector<3> geometricOrigin;
    bool isGeometricOriginZero{};

    [[nodiscard]] Matrix<3, 3> getRotationMatrixPretendingToBeReflection(const Shape &shape) const;
    [[nodiscard]] Vector<3> getReflectionAxisForShape(const Shape &shape) const;
    [[nodiscard]] Vector<3> getFlipSymmetryAxisForShape(const Shape &shape) const;

public:
    /**
     * @brief Constructs the class specifying how often to perform a reflection (i.e., how many moves should be
     * requested, calculated by dividing the number of molecules by @a flipEvery).
     */
    ReflectionSampler(const GeneralizedShapeAxis &reflectionAxis, const FlipSymmetryAxis &flipSymmetryAxis,
                      std::size_t flipEvery);

    [[nodiscard]] std::string getName() const override { return "reflection"; }
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
