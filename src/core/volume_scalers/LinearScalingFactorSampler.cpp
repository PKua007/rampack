//
// Created by pkua on 28.06.22.
//

#include "LinearScalingFactorSampler.h"
#include "utils/Assertions.h"


std::array<double, 3> LinearScalingFactorSampler::sampleFactors(std::array<bool, 3> shouldSample,
                                                                const std::array<double, 3> &boxHeights,
                                                                double stepSize, std::mt19937 &mt) const
{
    std::uniform_real_distribution<double> stepSizeDistribution(-stepSize, stepSize);
    double step = stepSizeDistribution(mt);
    std::array<double, 3> scalingFactors = {1, 1, 1};
    for (std::size_t i{}; i < 3; i++) {
        if (shouldSample[i]) {
            scalingFactors[i] = (boxHeights[i] + step) / boxHeights[i];
            Ensures(scalingFactors[i] > 0);
        }
    }
    return scalingFactors;
}
