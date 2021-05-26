//
// Created by Piotr Kubala on 22/03/2021.
//

#include <algorithm>
#include <numeric>

#include "LogVolumeScaler.h"

std::array<double, 3> LogVolumeScaler::sampleScalingFactors([[maybe_unused]] const std::array<double, 3> &oldDim,
                                                            double scalingStepSize, std::mt19937 &mt) const
{
    std::array<double, 3> independentScalingFactors = this->sampleIndependentScalingFactors(scalingStepSize, mt);
    std::array<double, 3> scalingFactors{};
    switch (this->scalingDirection) {
        case ScalingDirection::ISOTROPIC:
            scalingFactors[0] = independentScalingFactors[0];
            scalingFactors[1] = independentScalingFactors[0];
            scalingFactors[2] = independentScalingFactors[0];
            break;
        case ScalingDirection::ANISOTROPIC_X:
            scalingFactors[0] = independentScalingFactors[0];
            scalingFactors[1] = independentScalingFactors[1];
            scalingFactors[2] = independentScalingFactors[1];
            break;
        case ScalingDirection::ANISOTROPIC_Y:
            scalingFactors[0] = independentScalingFactors[1];
            scalingFactors[1] = independentScalingFactors[0];
            scalingFactors[2] = independentScalingFactors[1];
            break;
        case ScalingDirection::ANISOTROPIC_Z:
            scalingFactors[0] = independentScalingFactors[1];
            scalingFactors[1] = independentScalingFactors[1];
            scalingFactors[2] = independentScalingFactors[0];
            break;
        case ScalingDirection::ANISOTROPIC_XYZ:
            scalingFactors[0] = independentScalingFactors[0];
            scalingFactors[1] = independentScalingFactors[1];
            scalingFactors[2] = independentScalingFactors[2];
            break;
    }

    for (auto &scalingFactor : scalingFactors)
        scalingFactor = std::exp(scalingFactor);

    return scalingFactors;
}

std::array<double, 3> LogVolumeScaler::sampleIndependentScalingFactors(double scalingStepSize, std::mt19937 &mt) const {
    std::uniform_real_distribution<double> scalingLogDistribution(-scalingStepSize, scalingStepSize);
    auto scalingLogSampler = [&](){ return scalingLogDistribution(mt); };

    std::array<double, 3> independentScalingFactors{};
    independentScalingFactors.fill(0);

    if (this->scaleTogether) {
        std::generate(independentScalingFactors.begin(), independentScalingFactors.end(), scalingLogSampler);
    } else {
        std::uniform_int_distribution<std::size_t> zeroToOne(0, 1);
        std::uniform_int_distribution<std::size_t> zeroToTwo(0, 2);
        switch (this->scalingDirection) {
            case ScalingDirection::ISOTROPIC:
                independentScalingFactors[0] = scalingLogSampler();
                break;
            case ScalingDirection::ANISOTROPIC_X:
            case ScalingDirection::ANISOTROPIC_Y:
            case ScalingDirection::ANISOTROPIC_Z:
                independentScalingFactors[zeroToOne(mt)] = scalingLogSampler();
                break;
            case ScalingDirection::ANISOTROPIC_XYZ:
                independentScalingFactors[zeroToTwo(mt)] = scalingLogSampler();
                break;
        }
    }
    return independentScalingFactors;
}
