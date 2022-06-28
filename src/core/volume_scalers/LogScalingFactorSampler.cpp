//
// Created by pkua on 28.06.22.
//

#include "LogScalingFactorSampler.h"


std::array<double, 3> LogScalingFactorSampler::sampleFactors(std::array<bool, 3> shouldSample,
                                                             [[maybe_unused]] const std::array<double, 3> &boxHeights,
                                                             double stepSize, std::mt19937 &mt) const
{
    std::uniform_real_distribution<double> scalingLogDistribution(-stepSize, stepSize);
    double factor = std::exp(scalingLogDistribution(mt));
    std::array<double, 3> scalingFactors = {1, 1, 1};
    for (std::size_t i{}; i < 3; i++)
        if (shouldSample[i])
            scalingFactors[i] = factor;
    return scalingFactors;
}
