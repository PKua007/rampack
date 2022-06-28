//
// Created by pkua on 28.06.22.
//

#ifndef RAMPACK_LINEARSCALINGFACTORSAMPLER_H
#define RAMPACK_LINEARSCALINGFACTORSAMPLER_H

#include "ScalingFactorSampler.h"


class LinearScalingFactorSampler : public ScalingFactorSampler {
public:
    [[nodiscard]] std::array<double, 3> sampleFactors(std::array<bool, 3> shouldSample,
                                                      const std::array<double, 3> &boxHeights, double stepSize,
                                                      std::mt19937 &mt) const override;
};


#endif //RAMPACK_LINEARSCALINGFACTORSAMPLER_H
