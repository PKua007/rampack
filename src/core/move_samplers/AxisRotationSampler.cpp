//
// Created by ciesla on 09.06.2024.
//

#include "AxisRotationSampler.h"
#include "utils/Exceptions.h"


Vector<3> AxisRotationSampler::computeAxis(const Shape &shape) const {
    Vector<3> rotationAxis;

    if (std::holds_alternative<ShapeGeometry::Axis>(axis)) {
        const auto &shapeAxis = std::get<ShapeGeometry::Axis>(axis);
        rotationAxis = geometry->getAxis(shape, shapeAxis);
        return rotationAxis.normalized();
    } else if (std::holds_alternative<Vector<3>>(axis)) {
        const auto &globalAxis = std::get<Vector<3>>(axis);
        return globalAxis;
    } else { // valueless_by_exception
        AssertThrow("std::variant::valueless_by_exception");
    }
}

AxisRotationSampler::AxisRotationSampler(double rotationStepSize, const Axis &axis)
        : rotationStepSize{rotationStepSize}, axis{axis}
{
    Expects(this->rotationStepSize > 0);

    if (std::holds_alternative<Vector<3>>(this->axis)) {
        constexpr double EPSILON = 1e-12;
        auto &globalAxis = std::get<Vector<3>>(this->axis);

        Expects(globalAxis.norm2() > EPSILON * EPSILON);

        globalAxis = globalAxis.normalized();
    }
}

MoveSampler::MoveData AxisRotationSampler::sampleMove([[maybe_unused]] const Packing &packing,
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

bool AxisRotationSampler::increaseStepSize() {
    double oldRotationStepSize = this->rotationStepSize;

    this->rotationStepSize *= 1.1;
    if (this->rotationStepSize > M_PI)
        this->rotationStepSize = M_PI;

    return this->rotationStepSize != oldRotationStepSize;
}

bool AxisRotationSampler::decreaseStepSize() {
    this->rotationStepSize /= 1.1;
    return true;
}

std::vector<std::pair<std::string, double>> AxisRotationSampler::getStepSizes() const {
    return {{"rotation", this->rotationStepSize}};
}

void AxisRotationSampler::setStepSize(const std::string &stepName, double stepSize) {
    Expects(stepSize > 0);
    Expects(stepName == "rotation");

    this->rotationStepSize = stepSize;
}

std::string AxisRotationSampler::getName() const {
    if (std::holds_alternative<ShapeGeometry::Axis>(this->axis)) {
        const auto &shapeAxis = std::get<ShapeGeometry::Axis>(this->axis);
        switch (shapeAxis) {
            case ShapeGeometry::Axis::PRIMARY:
                return "primary_axis_rotation";
            case ShapeGeometry::Axis::SECONDARY:
                return "secondary_axis_rotation";
            case ShapeGeometry::Axis::AUXILIARY:
                return "auxiliary_axis_rotation";
            default:
                AssertThrow("ShapeGeometry::Axis");
        }
    } else if (std::holds_alternative<Vector<3>>(this->axis)) {
        const auto &globalAxis = std::get<Vector<3>>(this->axis);
        std::ostringstream nameOut;
        nameOut.precision(std::numeric_limits<double>::max_digits10);
        nameOut << "{" << globalAxis[0] << "," << globalAxis[1] << "," << globalAxis[2] << "}_axis_rotation";
        return nameOut.str();
    } else { // valueless_by_exception
        AssertThrow("std::variant::valueless_by_exception");
    }
}
