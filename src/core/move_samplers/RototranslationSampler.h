//
// Created by pkua on 18.03.2022.
//

#ifndef RAMPACK_ROTOTRANSLATIONSAMPLER_H
#define RAMPACK_ROTOTRANSLATIONSAMPLER_H

#include <optional>

#include "core/MoveSampler.h"


/**
 * @brief MoveSampler performing translational and rotation move at the same time.
 * @details Particles are sampled at random. The way of sampling and step sizes are the same as in TranslationSampler
 * and RotationSampler, so one is referred to their documentation. Internally it consists of a two moves named
 * @a translation and @a rotation. The group name is @a rototranslation.
 */
class RototranslationSampler : public MoveSampler {
private:
    double translationStepSize{};
    std::optional<double> rotationStepSize{};
    double maxTranslationStepSize{};
    bool isMaxTranslationStepSizeImplicit{};

    void adjustMaxTranslationStepSize(const Packing &packing);
    bool increaseTranslationStepSize();

public:
   /**
     * @brief Creates the sampler. If @a std::nullopt is passed to @a rotationStepSize, it is calculated using
     * heuristically found formula.
     */
    RototranslationSampler(double translationStepSize, std::optional<double> rotationStepSize,
                           double maxTranslationStepSize = 0);

    [[nodiscard]] std::string getName() const override { return "rototranslation"; }

    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override { return numParticles; }

    MoveData sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs,
                        std::mt19937 &mt) override;
    bool increaseStepSize() override;
    bool decreaseStepSize() override;
    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override;
    void setStepSize(const std::string &stepName, double stepSize) override;
    void setupForShapeTraits(const ShapeTraits &shapeTraits) override;
};


#endif //RAMPACK_ROTOTRANSLATIONSAMPLER_H
