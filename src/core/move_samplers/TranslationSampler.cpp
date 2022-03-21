//
// Created by pkua on 21.03.2022.
//

#include "TranslationSampler.h"

#include "utils/Assertions.h"


TranslationSampler::TranslationSampler(double translationStepSize, double maxTranslationStepSize)
        : translationStepSize{translationStepSize}, maxTranslationStepSize{maxTranslationStepSize}
{
    Expects(maxTranslationStepSize >= 0);
    if (maxTranslationStepSize > 0)
        Expects(translationStepSize <= maxTranslationStepSize);
    Expects(translationStepSize > 0);
}

MoveSampler::MoveData TranslationSampler::sampleMove([[maybe_unused]] const Packing &packing,
                                                     [[maybe_unused]] const ShapeTraits &shapeTraits,
                                                     const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt)
{
    using URD = std::uniform_real_distribution<double>;

    MoveData moveData;
    moveData.moveType = MoveType::TRANSLATION;

    URD translationDistribution(-this->translationStepSize, this->translationStepSize);
    moveData.translation = {translationDistribution(mt), translationDistribution(mt), translationDistribution(mt)};

    std::uniform_int_distribution<std::size_t> particleDistribution(0, particleIdxs.size() - 1);
    moveData.particleIdx = particleIdxs[particleDistribution(mt)];

    return moveData;
}

bool TranslationSampler::increaseStepSize() {
    double oldTranslationStepSize = this->translationStepSize;
    this->translationStepSize *= 1.1;
    if (this->maxTranslationStepSize > 0 && this->translationStepSize > this->maxTranslationStepSize)
        this->translationStepSize = this->maxTranslationStepSize;

    return this->translationStepSize != oldTranslationStepSize;
}

bool TranslationSampler::decreaseStepSize() {
    this->translationStepSize /= 1.1;
    return true;
}

std::vector<std::pair<std::string, double>> TranslationSampler::getStepSizes() const {
    return {{"translation", this->translationStepSize}};
}

void TranslationSampler::setStepSize(const std::string &stepName, double stepSize) {
    Expects(stepSize > 0);
    Expects(stepName == "translation");

    this->translationStepSize = stepSize;
}