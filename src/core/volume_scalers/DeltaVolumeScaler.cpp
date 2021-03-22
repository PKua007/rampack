//
// Created by Piotr Kubala on 22/03/2021.
//

#include <algorithm>
#include <numeric>

#include "DeltaVolumeScaler.h"

std::array<double, 3> DeltaVolumeScaler::sampleScalingFactors(const std::array<double, 3> &oldDim,
                                                              double scalingStepSize,
                                                              std::mt19937 &mt) const
{
    std::uniform_real_distribution<double> unitIntervalDistribution(0, 1);
    double deltaV = (2*unitIntervalDistribution(mt) - 1) * scalingStepSize;
    double currentV = std::accumulate(oldDim.begin(), oldDim.end(), 1., std::multiplies<>{});
    double factor = std::cbrt((deltaV + currentV) / currentV);
    return {factor, factor, factor};
}
