//
// Created by Piotr Kubala on 22/03/2021.
//

#include <algorithm>
#include <numeric>

#include "LogVolumeScaler.h"

std::array<double, 3> LogVolumeScaler::sampleScalingFactors([[maybe_unused]] const std::array<double, 3> &oldDim,
                                                            double scalingStepSize, std::mt19937 &mt) const
{
    std::array<double, 3> scalingFactors{};
    std::uniform_real_distribution<double> unitIntervalDistribution(0, 1);
    scalingFactors.fill((2 * unitIntervalDistribution(mt) - 1) * scalingStepSize);
    switch (this->scalingDirection) {
        case ScalingDirection::ISOTROPIC:
            break;
        case ScalingDirection::ANISOTROPIC_X:
            scalingFactors[0] = (2 * unitIntervalDistribution(mt) - 1) * scalingStepSize;
            break;
        case ScalingDirection::ANISOTROPIC_Y:
            scalingFactors[1] = (2 * unitIntervalDistribution(mt) - 1) * scalingStepSize;
            break;
        case ScalingDirection::ANISOTROPIC_Z:
            scalingFactors[2] = (2 * unitIntervalDistribution(mt) - 1) * scalingStepSize;
            break;
        case ScalingDirection::ANISOTROPIC_XYZ:
            scalingFactors[1] = (2 * unitIntervalDistribution(mt) - 1) * scalingStepSize;
            scalingFactors[2] = (2 * unitIntervalDistribution(mt) - 1) * scalingStepSize;
            break;
    }

    for (auto &scalingFactor : scalingFactors)
        scalingFactor = std::exp(scalingFactor);

    return scalingFactors;
}
