//
// Created by pkua on 21.03.2022.
//

#include "FlipSampler.h"
#include "utils/Exceptions.h"


FlipSampler::FlipSampler(size_t flipEvery) : flipEvery{flipEvery} {
    Expects(flipEvery > 0);
}

std::size_t FlipSampler::getNumOfRequestedMoves(std::size_t numParticles) const {
    Expects(numParticles > this->flipEvery);
    return numParticles / this->flipEvery;
}

MoveSampler::MoveData FlipSampler::sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs,
                                              std::mt19937 &mt)
{
    MoveData moveData;

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];

    const Shape &shape = packing[moveData.particleIdx];
    // Renormalize after rotation for better numerical stability
    Vector<3> shapeFlipAxis = (shape.getOrientation() * this->flipAxis).normalized();
    moveData.rotation = Matrix<3, 3>::rotation(shapeFlipAxis, M_PI);

    if (this->isGeometricOriginZero) {
        moveData.moveType = MoveType::ROTATION;
    } else {
        moveData.moveType = MoveType::ROTOTRANSLATION;
        // Restore original geometric origin position after the flip
        Vector<3> shapeGeometricOrigin = shape.getOrientation()*this->geometricOrigin;
        moveData.translation = -moveData.rotation * shapeGeometricOrigin + shapeGeometricOrigin;
    }

    return moveData;
}

void FlipSampler::setup([[maybe_unused]] const Packing &packing, const ShapeTraits &shapeTraits) {
    const auto &geometry = shapeTraits.getGeometry();
    Expects(geometry.hasPrimaryAxis());

    this->geometricOrigin = geometry.getGeometricOrigin({});
    constexpr double EPSILON = 1e-14;
    this->isGeometricOriginZero = (this->geometricOrigin.norm2() < EPSILON*EPSILON);

    this->flipAxis = geometry.findFlipAxis({});
}
