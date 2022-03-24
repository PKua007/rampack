//
// Created by pkua on 21.03.2022.
//

#ifndef RAMPACK_TRANSLATIONSAMPLER_H
#define RAMPACK_TRANSLATIONSAMPLER_H

#include "core/MoveSampler.h"


/**
 * @brief MoveSampler performing only the translational moves.
 * @details Particles are sampled at random and each coordinate of translation vector is sampled independently from a
 * uniform interval given by the current step size. Internally it consists of a single move named @a translation. The
 * group name is also @a translation.
 */
class TranslationSampler : public MoveSampler {
private:
    double translationStepSize{};
    double maxTranslationStepSize{};

public:
    /**
     * @brief Constructs the sampler.
     * @param translationStepSize the initial step size (+- endpoids of sampling interval)
     * @param maxTranslationStepSize maximal allowed value of the step size. If 0, no limit iwll be imposed.
     */
    explicit TranslationSampler(double translationStepSize, double maxTranslationStepSize = 0);

    [[nodiscard]] std::string getName() const override { return "translation"; }

    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override { return numParticles; }

    MoveData sampleMove(const Packing &packing, const ShapeTraits &shapeTraits,
                        const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt) override;
    bool increaseStepSize() override;
    bool decreaseStepSize() override;

    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override;

    void setStepSize(const std::string &stepName, double stepSize) override;
};


#endif //RAMPACK_TRANSLATIONSAMPLER_H
