//
// Created by ciesla on 09.06.2024.
//

#include "RotationWithAngleConservationSampler.h"
#include "utils/Exceptions.h"


RotationWithAngleConservationSampler::RotationWithAngleConservationSampler(double rotationStepSize, size_t particleAxisIdx,
                                                                           const Vector<3, double> &globalAxis) :
                                                                           rotationStepSize{rotationStepSize},
                                                                           particleAxisIdx{particleAxisIdx},
                                                                           globalAxis(globalAxis){
    Expects(rotationStepSize > 0);
    Expects(particleAxisIdx<3);
    Expects( (globalAxis[0]!=0 || globalAxis[1]!=0 || globalAxis[2] !=0) );
    globalAxis.normalized();
}

MoveSampler::MoveData RotationWithAngleConservationSampler::sampleMove([[maybe_unused]] const Packing &packing,
                                                  const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt)
{
    using URD = std::uniform_real_distribution<double>;

    MoveData moveData;
    moveData.moveType = MoveType::ROTATION;

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];

    URD plusMinusOneDistribution(-1, 1);
    Vector<3> axis;
    if (plusMinusOneDistribution(mt)>0){
        axis = this->globalAxis;
    }else{
        axis = packing[moveData.particleIdx].getOrientation()*this->particleAxis;
    }
    URD rotationAngleDistribution(-this->rotationStepSize, this->rotationStepSize);
    double angle = rotationAngleDistribution(mt);
    moveData.rotation = Matrix<3, 3>::rotation(axis.normalized(), angle);

    return moveData;
}

bool RotationWithAngleConservationSampler::increaseStepSize() {
    double oldRotationStepSize = this->rotationStepSize;

    this->rotationStepSize *= 1.1;
    if (this->rotationStepSize > M_PI)
        this->rotationStepSize = M_PI;

    return this->rotationStepSize != oldRotationStepSize;
}

bool RotationWithAngleConservationSampler::decreaseStepSize() {
    this->rotationStepSize /= 1.1;
    return true;
}

std::vector<std::pair<std::string, double>> RotationWithAngleConservationSampler::getStepSizes() const {
    return {{"rotationWithAngleConservation", this->rotationStepSize}};
}

void RotationWithAngleConservationSampler::setStepSize(const std::string &stepName, double stepSize) {
    Expects(stepSize > 0);
    Expects(stepName == "rotationWithAngleConservation");

    this->rotationStepSize = stepSize;
}

void RotationWithAngleConservationSampler::setupForShapeTraits(const ShapeTraits &shapeTraits) {
    const auto &geometry = shapeTraits.getGeometry();
    switch(this->particleAxisIdx) {
        case 2:
            Expects(geometry.hasAuxiliaryAxis());
            this->particleAxis = geometry.getAuxiliaryAxis({});
            break;
        case 1:
            Expects(geometry.hasSecondaryAxis());
            this->particleAxis = geometry.getSecondaryAxis({});
            break;
        default:
            Expects(geometry.hasPrimaryAxis());
            this->particleAxis = geometry.getPrimaryAxis({});
            break;
    }
}
