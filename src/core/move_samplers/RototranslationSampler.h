//
// Created by pkua on 18.03.2022.
//

#ifndef RAMPACK_ROTOTRANSLATIONSAMPLER_H
#define RAMPACK_ROTOTRANSLATIONSAMPLER_H

#include "core/MoveSampler.h"


class RototranslationSampler : public MoveSampler {
private:
    double translationStepSize{};
    double rotationStepSize{};
    double maxTranslationStepSize{};

public:
    RototranslationSampler(double translationStepSize, double rotationStepSize, double maxTranslationStepSize = 0);

    [[nodiscard]] std::string getName() const override { return "rototranslation"; }

    MoveData sampleMove(const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt) override;
    bool increaseStepSize() override;
    bool decreaseStepSize() override;

    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override;
};


#endif //RAMPACK_ROTOTRANSLATIONSAMPLER_H
