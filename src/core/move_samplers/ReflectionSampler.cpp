//
// Created by ciesla on 1/1/25.
//

#include "ReflectionSampler.h"

ReflectionSampler::ReflectionSampler(std::size_t flipEvery, const Vector<3> &plane) : flipEvery{flipEvery}, planeAxis{plane.normalized()}{
    constexpr double EPSILON = 1e-12;
    Expects(plane.norm2() > EPSILON * EPSILON);
    Expects(flipEvery > 0);
}

std::size_t ReflectionSampler::getNumOfRequestedMoves(std::size_t numParticles) const {
    Expects(numParticles > this->flipEvery);
    return numParticles / this->flipEvery;
}

Matrix<3, 3, double> ReflectionSampler::getRotationMatrix(const Vector<3, double> &axis, double cosangle)
{
    double sina = -2*cosangle*std::sqrt(1-cosangle*cosangle);
    double cosa = 1-2*cosangle*cosangle;
    Matrix<3, 3> K = {{      0 , -axis[2],  axis[1],
                             axis[2],        0, -axis[0],
                             -axis[1],  axis[0],        0}};

    // Rodrigues' rotation formula
    return Matrix<3, 3>::identity() + K*sina + (1 - cosa)*K*K;
}


MoveSampler::MoveData ReflectionSampler::sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt){
    Expects(this->geometry != nullptr);

    MoveData moveData;
    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];
    Shape shape = packing[moveData.particleIdx];

    Vector<3> shapeAxis = geometry->getPrimaryAxis(shape);
    Vector<3> rotationAxis = (shapeAxis^this->planeAxis).normalized();

    double cosangle = shapeAxis*this->planeAxis;
    moveData.rotation = this->getRotationMatrix(rotationAxis, cosangle);
    moveData.moveType = MoveType::ROTATION;
    return moveData;
}


