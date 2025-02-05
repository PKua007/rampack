//
// Created by ciesla on 1/1/25.
//

#include "ReflectionSampler.h"

ReflectionSampler::ReflectionSampler(std::size_t flipEvery, Vector<3> plane) : flipEvery{flipEvery}, planeAxis{plane.normalized()}{
    Expects(flipEvery > 0);
}

std::size_t ReflectionSampler::getNumOfRequestedMoves(std::size_t numParticles) const {
    Expects(numParticles > this->flipEvery);
    return numParticles / this->flipEvery;
}

MoveSampler::MoveData ReflectionSampler::sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs,
                                              std::mt19937 &mt){
    Expects(this->geometry != nullptr);

    MoveData moveData;
    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];
    Shape shape = packing[moveData.particleIdx];

    Vector<3> shapeAxis = geometry->getPrimaryAxis(shape);
    double angle = std::acos(shapeAxis*this->planeAxis);
    double rotationAngle = M_PI - 2*angle;
    Vector<3> rotationAxis = (shapeAxis^this->planeAxis).normalized();
    moveData.rotation = Matrix<3, 3>::rotation(rotationAxis, -rotationAngle);
    moveData.moveType = MoveType::ROTATION;
    return moveData;
}


