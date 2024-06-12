//
// Created by ciesla on 09.06.2024.
//

#include "RotationAroundAxisSampler.h"
#include "utils/Exceptions.h"


RotationAroundAxisSampler::RotationAroundAxisSampler(double rotationStepSize,
                                                                           const Vector<3, double> &axis,
                                                                           bool global) :
                                                                           rotationStepSize{rotationStepSize},
                                                                           axis{axis},
                                                                           global{global}{
    Expects(rotationStepSize > 0);
    Expects( (axis[0]!=0 || axis[1]!=0 || axis[2] !=0) );
    axis.normalized();
}

MoveSampler::MoveData RotationAroundAxisSampler::sampleMove([[maybe_unused]] const Packing &packing,
                                                  const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt)
{
    using URD = std::uniform_real_distribution<double>;

    MoveData moveData;
    moveData.moveType = MoveType::ROTATION;

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];

    Vector<3> rotationAxis;
    if (this->global){
        rotationAxis = this->axis;
    }else{
        rotationAxis = packing[moveData.particleIdx].getOrientation()*this->axis;
    }
    URD rotationAngleDistribution(-this->rotationStepSize, this->rotationStepSize);
    double angle = rotationAngleDistribution(mt);
    moveData.rotation = Matrix<3, 3>::rotation(rotationAxis.normalized(), angle);

    return moveData;
}

bool RotationAroundAxisSampler::increaseStepSize() {
    double oldRotationStepSize = this->rotationStepSize;

    this->rotationStepSize *= 1.1;
    if (this->rotationStepSize > M_PI)
        this->rotationStepSize = M_PI;

    return this->rotationStepSize != oldRotationStepSize;
}

bool RotationAroundAxisSampler::decreaseStepSize() {
    this->rotationStepSize /= 1.1;
    return true;
}

std::vector<std::pair<std::string, double>> RotationAroundAxisSampler::getStepSizes() const {
    return {{"rotationAroundAxis", this->rotationStepSize}};
}

void RotationAroundAxisSampler::setStepSize(const std::string &stepName, double stepSize) {
    Expects(stepSize > 0);
    Expects(stepName == "rotationAroundAxis");

    this->rotationStepSize = stepSize;
}