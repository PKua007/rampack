//
// Created by pkua on 21.03.2022.
//

#include "FlipSampler.h"
#include "utils/Assertions.h"


FlipSampler::FlipSampler(size_t flipEvery) : flipEvery{flipEvery} {
    Expects(flipEvery > 0);
}

std::size_t FlipSampler::getNumOfRequestedMoves(std::size_t numParticles) const {
    Expects(numParticles > this->flipEvery);
    return numParticles / this->flipEvery;
}

MoveSampler::MoveData FlipSampler::sampleMove(const Packing &packing, const ShapeTraits &shapeTraits,
                                              const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt)
{
    MoveData moveData;
    moveData.moveType = MoveType::ROTATION;

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];

    const Shape &shape = packing[moveData.particleIdx];
    Vector<3> axis = shapeTraits.getSecondaryAxis(shape);
    moveData.rotation = Matrix<3, 3>::rotation(axis, M_PI);

    return moveData;
}