//
// Created by pkua on 21.03.2022.
//

#include "RotationSampler.h"
#include "utils/Assertions.h"


RotationSampler::RotationSampler(double rotationStepSize) : rotationStepSize{rotationStepSize} {
    Expects(rotationStepSize > 0);
}

MoveSampler::MoveData RotationSampler::sampleMove([[maybe_unused]] const Packing &packing,
                                                  [[maybe_unused]] const ShapeTraits &shapeTraits,
                                                  const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt)
{
    using URD = std::uniform_real_distribution<double>;

    MoveData moveData;
    moveData.moveType = MoveType::ROTATION;

    URD rotationAngleDistribution(-this->rotationStepSize, this->rotationStepSize);
    URD plusMinusOneDistribution(-1, 1);
    Vector<3> axis;
    do {
        axis = {plusMinusOneDistribution(mt), plusMinusOneDistribution(mt), plusMinusOneDistribution(mt)};
    } while (axis.norm2() > 1);
    double angle = rotationAngleDistribution(mt);
    moveData.rotation = Matrix<3, 3>::rotation(axis.normalized(), angle);

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];

    return moveData;
}

bool RotationSampler::increaseStepSize() {
    double oldRotationStepSize = this->rotationStepSize;

    this->rotationStepSize *= 1.1;
    if (this->rotationStepSize > M_PI)
        this->rotationStepSize = M_PI;

    return this->rotationStepSize != oldRotationStepSize;
}

bool RotationSampler::decreaseStepSize() {
    this->rotationStepSize /= 1.1;
    return true;
}

std::vector<std::pair<std::string, double>> RotationSampler::getStepSizes() const {
    return {{"rotation", this->rotationStepSize}};
}

void RotationSampler::setStepSize(const std::string &stepName, double stepSize) {
    Expects(stepSize > 0);
    Expects(stepName == "rotation");

    this->rotationStepSize = stepSize;
}