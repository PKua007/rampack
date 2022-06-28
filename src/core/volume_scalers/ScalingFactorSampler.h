//
// Created by pkua on 27.06.22.
//

#ifndef RAMPACK_SCALINGFACTORSAMPLER_H
#define RAMPACK_SCALINGFACTORSAMPLER_H

#include <array>
#include <random>


class ScalingFactorSampler {
public:
    virtual ~ScalingFactorSampler() = default;

    [[nodiscard]] virtual std::array<double, 3> sampleFactors(std::array<bool, 3> shouldSample,
                                                              const std::array<double, 3> &boxHeights,
                                                              double stepSize, std::mt19937 &mt) const = 0;
};


#endif //RAMPACK_SCALINGFACTORSAMPLER_H
