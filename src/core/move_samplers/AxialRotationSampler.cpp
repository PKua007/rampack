//
// Created by ciesla on 09.06.2024.
//

#include "AxialRotationSampler.h"
#include <limits>
#include <sstream>

#include "utils/Exceptions.h"


Vector<3> AxialRotationSampler::computeAxis(const Shape &shape) const {
    if (const auto *labAxis = std::get_if<Vector<3>>(&this->axis))
        return *labAxis;
    if (const auto *shapeAxis = std::get_if<GeneralShapeAxis>(&this->axis))
        return shapeAxis->getForShape(*this->geometry, shape).normalized();
    AssertThrow("std::variant::valueless_by_exception");
}

AxialRotationSampler::AxialRotationSampler(const double rotationStepSize, const GeneralShapeAxis &axis)
        : rotationStepSize{rotationStepSize}, axis{axis}
{
    Expects(this->rotationStepSize > 0);
}

AxialRotationSampler::AxialRotationSampler(const double rotationStepSize, const Vector<3> &axis)
        : rotationStepSize{rotationStepSize}, axis{axis.normalized()}
{
    constexpr double EPSILON = 1e-12;
    Expects(this->rotationStepSize > 0);
    Expects(axis.norm2() > EPSILON * EPSILON);
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

std::string AxialRotationSampler::getAxisNameSuffix() const {
    if (const auto *labAxis = std::get_if<Vector<3>>(&this->axis)) {
        std::ostringstream nameOut;
        nameOut.precision(std::numeric_limits<double>::max_digits10);
        nameOut << (*labAxis)[0] << "," << (*labAxis)[1] << "," << (*labAxis)[2];
        return nameOut.str();
    }
    if (const auto *shapeAxis = std::get_if<GeneralShapeAxis>(&this->axis))
        return shapeAxis->getMoveSamplerNameSuffix();
    AssertThrow("std::variant::valueless_by_exception");
}

std::string AxialRotationSampler::getName() const {
    return "axial_rotation(" + this->getAxisNameSuffix() + ")";
}
