//
// Created by ciesla on 1/1/25.
//

#include "ReflectionSampler.h"

ReflectionSampler::ReflectionSampler(const GeneralizedShapeAxis &reflectionAxis_,
                                     const FlipAxis &flipSymmetryAxis_, const std::size_t flipEvery_)
        : reflectionAxis{reflectionAxis_}, flipSymmetryAxis{flipSymmetryAxis_},
          flipEvery{flipEvery_}
{
    Expects(flipEvery_ > 0);
}

std::size_t ReflectionSampler::getNumOfRequestedMoves(std::size_t numParticles) const {
    Expects(numParticles > this->flipEvery);
    return numParticles / this->flipEvery;
}

void ReflectionSampler::setupForShapeTraits(const ShapeTraits &shapeTraits) {
    const auto &geometry = shapeTraits.getGeometry();
    this->reflectionAxisForCurrentGeometry = this->reflectionAxis.getForDefaultOrientation(geometry);
    this->symmetryPlaneAxisForCurrentGeometry = this->flipSymmetryAxis.getForDefaultOrientation(geometry);

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
    moveData.rotation = this->getRotationMatrixPretendingToBeReflection(shape);

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

Matrix<3, 3> ReflectionSampler::getRotationMatrixPretendingToBeReflection(const Shape &shape) const {
    const Vector<3> reflectionAxisForShape = shape.getOrientation() * this->reflectionAxisForCurrentGeometry;
    const Vector<3> symmetryPlaneAxisForShape = shape.getOrientation() * this->symmetryPlaneAxisForCurrentGeometry;

    const double c = symmetryPlaneAxisForShape * reflectionAxisForShape;
    const Vector<3> v = symmetryPlaneAxisForShape ^ reflectionAxisForShape;
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

    // This is the composition of two reflections Ra * Rb, first through shape's symmetry plane axis `b`, then through
    // reflection plane axis `a`. The resulting rotation is around the axis (`a` x `b`) by the angle 2*`theta` given by
    // cos(`theta`) = `a` . `b`. The formula was optimized for a number of multiplications and additions with the
    // assistance of GPT o3.
    return {
        g + dv1v1,   dv1v2 - tv3, dv1v3 + tv2,
        dv1v2 + tv3, g + dv2v2,   dv2v3 - tv1,
        dv1v3 - tv2, dv2v3 + tv1, g + dv3v3
    };
}
