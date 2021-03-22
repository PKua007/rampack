//
// Created by Piotr Kubala on 22/03/2021.
//

#include <algorithm>
#include <numeric>

#include "LinearVolumeScaler.h"

std::array<double, 3> LinearVolumeScaler::sampleScalingFactors(const std::array<double, 3> &oldDim,
                                                               double scalingStepSize,
                                                               std::mt19937 &mt) const
{
    double oldV = std::accumulate(oldDim.begin(), oldDim.end(), 1., std::multiplies<>{});
    double meanDim = std::cbrt(oldV);
    double cbrtStep = std::cbrt(scalingStepSize);

    std::array<double, 3> deltaDim{};
    std::uniform_real_distribution<double> unitIntervalDistribution(0, 1);
    deltaDim.fill((2 * unitIntervalDistribution(mt) - 1) * cbrtStep);
    switch (this->scalingDirection) {
        case ScalingDirection::ISOTROPIC:
            break;
        case ScalingDirection::ANISOTROPIC_X:
            deltaDim[0] = (2 * unitIntervalDistribution(mt) - 1) * cbrtStep;
            break;
        case ScalingDirection::ANISOTROPIC_Y:
            deltaDim[1] = (2 * unitIntervalDistribution(mt) - 1) * cbrtStep;
            break;
        case ScalingDirection::ANISOTROPIC_Z:
            deltaDim[2] = (2 * unitIntervalDistribution(mt) - 1) * cbrtStep;
            break;
        case ScalingDirection::ANISOTROPIC_XYZ:
            deltaDim[1] = (2 * unitIntervalDistribution(mt) - 1) * cbrtStep;
            deltaDim[2] = (2 * unitIntervalDistribution(mt) - 1) * cbrtStep;
            break;
    }

    auto deltaDimScaler = [meanDim](double delta, double dim) { return delta * dim / meanDim; };
    std::transform(deltaDim.begin(), deltaDim.end(), oldDim.begin(), deltaDim.begin(), deltaDimScaler);

    std::array<double, 3> newDim{};
    std::transform(oldDim.begin(), oldDim.end(), deltaDim.begin(), newDim.begin(), std::plus<>{});
    std::array<double, 3> scalingFactor{};
    std::transform(newDim.begin(), newDim.end(), oldDim.begin(), scalingFactor.begin(), std::divides<>{});

    return scalingFactor;
}
