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

MoveSampler::MoveData RototranslationSampler::sampleMove([[maybe_unused]] const Packing &packing,
                                                         [[maybe_unused]] const ShapeTraits &shapeTraits,
                                                         const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt)
{
    using URD = std::uniform_real_distribution<double>;

    MoveData moveData;
    moveData.moveType = MoveType::ROTOTRANSLATION;

    URD translationDistribution(-this->translationStepSize, this->translationStepSize);
    moveData.translation = {translationDistribution(mt), translationDistribution(mt), translationDistribution(mt)};

    double rotationStepSize_ = std::min(this->rotationStepSize, M_PI);
    URD rotationAngleDistribution(-rotationStepSize_, rotationStepSize_);
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
    this->translationStepSize *= 1.1;
    if (this->maxTranslationStepSize > 0 && this->translationStepSize > this->maxTranslationStepSize)
        this->translationStepSize = this->maxTranslationStepSize;

    this->rotationStepSize *= this->translationStepSize/oldTranslationStepSize;

    return this->translationStepSize != oldTranslationStepSize;
}

bool RototranslationSampler::decreaseStepSize() {
    this->translationStepSize /= 1.1;
    this->rotationStepSize /= 1.1;
    return true;
}

std::vector<std::pair<std::string, double>> RototranslationSampler::getStepSizes() const {
    return {{"translation", this->translationStepSize}, {"rotation", this->rotationStepSize}};
}

void RototranslationSampler::setStepSize(const std::string &stepName, double stepSize) {
    Expects(stepSize > 0);
    if (stepName == "translation")
        this->translationStepSize = stepSize;
    else if (stepName == "rotation")
        this->rotationStepSize = stepSize;
    else
        throw PreconditionException("Unknown step name: " + stepName);
}

RototranslationSampler::RototranslationSampler(const Interaction &interaction, double translationStepSize,
                                               double maxTranslationStepSize)
        : translationStepSize{translationStepSize},
          maxTranslationStepSize{maxTranslationStepSize}
{
    Expects(maxTranslationStepSize >= 0);
    if (maxTranslationStepSize > 0)
        Expects(translationStepSize <= maxTranslationStepSize);
    Expects(translationStepSize > 0);
    Expects(interaction.getTotalRangeRadius() > 0);
    Expects(interaction.getTotalRangeRadius() < std::numeric_limits<double>::infinity());

    // Optimal rotation step size can be computed automatically based on translationStepSize and total interaction
    // radius. It was done by a following heuristic procedure:
    //
    // We carried out simulations for: Spherocylinder, SmoothWedge, PolyspherocylinderBanana (with 2 segments) and a
    // cube (using GenericXenoCollide) with various aspect ratios around packing fraction 0.5. In the simulations,
    // translational and rotational moves were separate and both step sizes were optimized individually. We then
    // fit the best quadratic function to rotationStepSize(translationStepSize/totalRangeRadius) dependence.

    double ratio = translationStepSize / interaction.getTotalRangeRadius();
    this->rotationStepSize = 0.133183 - 1.18634*ratio + 264.37*ratio*ratio;
    Assert(this->rotationStepSize > 0);
}
