//
// Created by pkua on 18.03.2022.
//

#include "RototranslationSampler.h"
#include "utils/Assertions.h"


RototranslationSampler::RototranslationSampler(double translationStepSize, double rotationStepSize,
                                               double maxTranslationStepSize)
        : translationStepSize{translationStepSize}, rotationStepSize{rotationStepSize},
          maxTranslationStepSize{maxTranslationStepSize}
{
    Expects(maxTranslationStepSize >= 0);
    if (maxTranslationStepSize > 0)
        Expects(translationStepSize <= maxTranslationStepSize);
    Expects(translationStepSize > 0);
    Expects(rotationStepSize > 0);
}

MoveSampler::MoveData RototranslationSampler::sampleMove(const std::vector<std::size_t> &particleIdxs,
                                                         std::mt19937 &mt)
{
    using URD = std::uniform_real_distribution<double>;

    MoveData moveData;
    moveData.moveType = MoveType::ROTOTRANSLATION;

    URD translationDistribution(-this->translationStepSize, this->translationStepSize);
    moveData.translation = {translationDistribution(mt), translationDistribution(mt), translationDistribution(mt)};

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

bool RototranslationSampler::increaseStepSize() {
    double oldTranslationStepSize = this->translationStepSize;
    double oldRotationStepSize = this->rotationStepSize;

    this->translationStepSize *= 1.1;
    if (this->maxTranslationStepSize > 0 && this->translationStepSize > this->maxTranslationStepSize)
        this->translationStepSize = this->maxTranslationStepSize;

    this->rotationStepSize *= 1.1;
    if (this->rotationStepSize > M_PI)
        this->rotationStepSize = M_PI;

    return (this->translationStepSize != oldTranslationStepSize || this->rotationStepSize != oldRotationStepSize);
}

bool RototranslationSampler::decreaseStepSize() {
    this->translationStepSize /= 1.1;
    this->rotationStepSize /= 1.1;
    return true;
}

std::vector<std::pair<std::string, double>> RototranslationSampler::getStepSizes() const {
    return {{"translation", this->translationStepSize}, {"rotation", this->rotationStepSize}};
}
