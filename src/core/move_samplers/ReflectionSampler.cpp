//
// Created by ciesla on 1/1/25.
//

#include "ReflectionSampler.h"

#include <limits>
#include <sstream>

ReflectionSampler::ReflectionSampler(const GeneralShapeAxis &reflectionAxis,
                                     const GeneralShapeAxis &reflectionSymmetryAxis, const std::size_t reflectEvery)
        : reflectionAxis{reflectionAxis}, reflectionSymmetryAxis{reflectionSymmetryAxis},
          reflectEvery{reflectEvery}
{
    Expects(reflectEvery > 0);
}

ReflectionSampler::ReflectionSampler(const Vector<3> &reflectionAxis,
                                     const GeneralShapeAxis &reflectionSymmetryAxis, const std::size_t reflectEvery)
        : reflectionAxis{reflectionAxis.normalized()}, reflectionSymmetryAxis{reflectionSymmetryAxis},
          reflectEvery{reflectEvery}
{
    constexpr double EPSILON = 1e-12;
    Expects(reflectionAxis.norm2() > EPSILON * EPSILON);
    Expects(reflectEvery > 0);
}

std::size_t ReflectionSampler::getNumOfRequestedMoves(const std::size_t numParticles) const {
    Expects(numParticles > this->reflectEvery);
    return numParticles / this->reflectEvery;
}

void ReflectionSampler::setupForShapeTraits(const ShapeTraits &shapeTraits) {
    const auto &geometry = shapeTraits.getGeometry();
    if (const auto *labAxis = std::get_if<Vector<3>>(&this->reflectionAxis))
        this->reflectionAxisForCurrentGeometry = *labAxis;
    else if (const auto *shapeAxis = std::get_if<GeneralShapeAxis>(&this->reflectionAxis))
        this->reflectionAxisForCurrentGeometry = shapeAxis->getForDefaultOrientation(geometry);
    else
        AssertThrow("std::variant::valueless_by_exception");

    this->symmetryPlaneAxisForCurrentGeometry = this->reflectionSymmetryAxis.getForDefaultOrientation(geometry);

    this->geometricOrigin = geometry.getGeometricOrigin(Shape{});
    constexpr double EPSILON = 1e-12;
    this->isGeometricOriginZero = this->geometricOrigin.norm2() < EPSILON*EPSILON;
}

MoveSampler::MoveData ReflectionSampler::sampleMove(const Packing &packing,
                                                    const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt)
{
    MoveData moveData;

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];

    const Shape &shape = packing[moveData.particleIdx];
    const Vector<3> reflectionAxisForShape = this->prepareReflectionAxis(shape);
    const Vector<3> symmetryPlaneAxisForShape = shape.getOrientation() * this->symmetryPlaneAxisForCurrentGeometry;
    moveData.rotation = ReflectionSampler::getRotationMatrixPretendingToBeReflection(reflectionAxisForShape,
                                                                                     symmetryPlaneAxisForShape);

    if (this->isGeometricOriginZero) {
        moveData.moveType = MoveType::ROTATION;
    } else {
        moveData.moveType = MoveType::ROTOTRANSLATION;
        // Restore the original geometric origin position after the flip
        const Vector<3> shapeGeometricOrigin = shape.getOrientation()*this->geometricOrigin;
        moveData.translation = -moveData.rotation * shapeGeometricOrigin + shapeGeometricOrigin;
    }

    return moveData;
}

Vector<3> ReflectionSampler::prepareReflectionAxis(const Shape &shape) const {
    if (std::holds_alternative<Vector<3>>(this->reflectionAxis))
        return this->reflectionAxisForCurrentGeometry;
    if (std::holds_alternative<GeneralShapeAxis>(this->reflectionAxis))
        return shape.getOrientation() * this->reflectionAxisForCurrentGeometry;
    AssertThrow("std::variant::valueless_by_exception");
}

std::string ReflectionSampler::getReflectionAxisNameSuffix() const {
    if (const auto *labAxis = std::get_if<Vector<3>>(&this->reflectionAxis)) {
        std::ostringstream nameOut;
        nameOut.precision(std::numeric_limits<double>::max_digits10);
        nameOut << (*labAxis)[0] << "," << (*labAxis)[1] << "," << (*labAxis)[2];
        return nameOut.str();
    }
    if (const auto *shapeAxis = std::get_if<GeneralShapeAxis>(&this->reflectionAxis))
        return shapeAxis->getMoveSamplerNameSuffix();
    AssertThrow("std::variant::valueless_by_exception");
}

Matrix<3, 3> ReflectionSampler::getRotationMatrixPretendingToBeReflection(const Vector<3> &reflectionAxis,
                                                                          const Vector<3> &symmetryPlaneAxis)
{
    const double c = symmetryPlaneAxis * reflectionAxis;
    const Vector<3> v = symmetryPlaneAxis ^ reflectionAxis;
    const double t = 2*c;
    const double g = t*c - 1;

    const double dv1v1 = 2*v[0]*v[0];
    const double dv2v2 = 2*v[1]*v[1];
    const double dv3v3 = 2*v[2]*v[2];
    const double dv1v2 = 2*v[0]*v[1];
    const double dv1v3 = 2*v[0]*v[2];
    const double dv2v3 = 2*v[1]*v[2];
    const double tv1 = t*v[0];
    const double tv2 = t*v[1];
    const double tv3 = t*v[2];

    // This is the composition of two reflections `R_a * R_b`, first through shape's symmetry plane axis `b`, then
    // through reflection plane axis `a`. The resulting rotation is around the axis `a x b` by the angle `2 * theta`,
    // where `cos(theta) = a . b`. The formula was optimized for a number of multiplications and additions with the
    // assistance of GPT o3, and debugged with the assistance of Codex 5.3.
    return {
        g + dv1v1,   dv1v2 - tv3, dv1v3 + tv2,
        dv1v2 + tv3, g + dv2v2,   dv2v3 - tv1,
        dv1v3 - tv2, dv2v3 + tv1, g + dv3v3
    };
}

std::string ReflectionSampler::getName() const {
    return "reflection(" + this->getReflectionAxisNameSuffix() + ")";
}
