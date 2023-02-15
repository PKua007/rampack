//
// Created by pkua on 18.03.2022.
//

#include "RototranslationSampler.h"
#include "utils/Exceptions.h"


RototranslationSampler::RototranslationSampler(double translationStepSize, std::optional<double> rotationStepSize,
                                               double maxTranslationStepSize)
        : translationStepSize{translationStepSize}, rotationStepSize{rotationStepSize},
          maxTranslationStepSize{maxTranslationStepSize}, isMaxTranslationStepSizeImplicit{maxTranslationStepSize == 0}
{
    Expects(maxTranslationStepSize >= 0);
    if (maxTranslationStepSize > 0)
        Expects(translationStepSize <= maxTranslationStepSize);
    Expects(translationStepSize > 0);
    if (rotationStepSize.has_value())
        Expects(rotationStepSize > 0);
}

MoveSampler::MoveData RototranslationSampler::sampleMove([[maybe_unused]] const Packing &packing,
                                                         const ShapeTraits &shapeTraits,
                                                         const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt)
{
    using URD = std::uniform_real_distribution<double>;

    MoveData moveData;
    moveData.moveType = MoveType::ROTOTRANSLATION;

    this->adjustMaxTranslationStepSize(packing);
    double actualTranslationStepSize = std::min(this->maxTranslationStepSize, this->translationStepSize);
    URD translationDistribution(-actualTranslationStepSize, actualTranslationStepSize);
    moveData.translation = {translationDistribution(mt), translationDistribution(mt), translationDistribution(mt)};

    this->calculateRotationStepSizeIfNeeded(shapeTraits.getInteraction());
    double rotationStepSize_ = std::min(*this->rotationStepSize, M_PI);
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
    bool wasIncreased = this->increaseTranslationStepSize();
    *this->rotationStepSize *= this->translationStepSize/oldTranslationStepSize;
    return wasIncreased;
}

bool RototranslationSampler::decreaseStepSize() {
    this->translationStepSize /= 1.1;
    *this->rotationStepSize /= 1.1;
    return true;
}

std::vector<std::pair<std::string, double>> RototranslationSampler::getStepSizes() const {
    return {{"translation", this->translationStepSize}, {"rotation", *this->rotationStepSize}};
}

void RototranslationSampler::setStepSize(const std::string &stepName, double stepSize) {
    Expects(stepSize > 0);
    if (stepName == "translation")
        this->translationStepSize = stepSize;
    else if (stepName == "rotation")
        this->rotationStepSize = stepSize;
    else
        ExpectsThrow("Unknown step name: " + stepName);
}

void RototranslationSampler::calculateRotationStepSizeIfNeeded(const Interaction &interaction) {
    if (this->rotationStepSize.has_value())
        return;

    Expects(interaction.getTotalRangeRadius() > 0);
    Expects(interaction.getTotalRangeRadius() < std::numeric_limits<double>::infinity());

    // Optimal rotation step size can be computed automatically based on translationStepSize and total interaction
    // radius. It was done by a following heuristic procedure:
    //
    // We carried out simulations for: Spherocylinder, SmoothWedge, PolyspherocylinderBanana (with 2 segments) and a
    // cube (using GenericXenoCollide) with various aspect ratios around packing fraction 0.5. In the simulations,
    // translational and rotational moves were separate and both step sizes were optimized individually. We then
    // fit the best quadratic function to rotationStepSize(translationStepSize/totalRangeRadius) dependence.

    double ratio = this->translationStepSize / interaction.getTotalRangeRadius();
    this->rotationStepSize = 0.133183 - 1.18634*ratio + 264.37*ratio*ratio;
    Assert(this->rotationStepSize > 0);
}

void RototranslationSampler::adjustMaxTranslationStepSize(const Packing &packing) {
    if (!this->isMaxTranslationStepSizeImplicit)
        return;

    auto heights = packing.getBox().getHeights();
    double minHeight = *std::min_element(heights.begin(), heights.end());
    this->maxTranslationStepSize = minHeight / 2;
}

bool RototranslationSampler::increaseTranslationStepSize() {
    if (this->maxTranslationStepSize > 0 && this->translationStepSize >= this->maxTranslationStepSize)
        return false;

    this->translationStepSize *= 1.1;
    return true;
}

