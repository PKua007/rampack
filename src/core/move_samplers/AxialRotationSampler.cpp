//
// Created by ciesla on 09.06.2024.
//

#include "AxialRotationSampler.h"
#include "utils/Exceptions.h"


Vector<3> AxialRotationSampler::computeAxis(const Shape &shape) const {
    return this->axis.getForShape(*this->geometry, shape).normalized();
}

AxialRotationSampler::AxialRotationSampler(const double rotationStepSize, const GeneralShapeAxis &axis)
        : rotationStepSize{rotationStepSize}, axis{axis}
{
    Expects(this->rotationStepSize > 0);
}

MoveSampler::MoveData AxialRotationSampler::sampleMove([[maybe_unused]] const Packing &packing,
                                                      const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt)
{
    Expects(this->geometry != nullptr);

    MoveData moveData;
    moveData.moveType = MoveType::ROTATION;

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];

    Vector<3> rotationAxis = this->computeAxis(packing[moveData.particleIdx]);
    std::uniform_real_distribution<double> rotationAngleDistribution(-this->rotationStepSize,
                                                                     this->rotationStepSize);
    double angle = rotationAngleDistribution(mt);
    moveData.rotation = Matrix<3, 3>::rotation(rotationAxis, angle);

    return moveData;
}

bool AxialRotationSampler::increaseStepSize() {
    const double oldRotationStepSize = this->rotationStepSize;

    this->rotationStepSize *= 1.1;
    if (this->rotationStepSize > M_PI)
        this->rotationStepSize = M_PI;

    return this->rotationStepSize != oldRotationStepSize;
}

bool AxialRotationSampler::decreaseStepSize() {
    this->rotationStepSize /= 1.1;
    return true;
}

std::vector<std::pair<std::string, double>> AxialRotationSampler::getStepSizes() const {
    return {{"rotation", this->rotationStepSize}};
}

void AxialRotationSampler::setStepSize(const std::string &stepName, double stepSize) {
    Expects(stepSize > 0);
    Expects(stepName == "rotation");

    this->rotationStepSize = stepSize;
}

std::string AxialRotationSampler::getName() const {
    return "axial_rotation(" + this->axis.getMoveSamplerNameSuffix() + ")";
}
