//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_LINEARVOLUMESCALER_H
#define RAMPACK_LINEARVOLUMESCALER_H

#include "core/VolumeScaler.h"

/**
 * @brief A VolumeScaler sampling box side length changes from a uniform interval with endpoints given by
 * +/- @a scalingStepSize.
 */
class LinearVolumeScaler : public VolumeScaler {
private:
    ScalingDirection scalingDirection{};

public:
    /**
     * @brief Construct the scaler with independent scaling directions given by @a scalingDirection.
     */
    explicit LinearVolumeScaler(ScalingDirection scalingDirection) : scalingDirection{scalingDirection} { }

    std::array<double, 3> sampleScalingFactors(const std::array<double, 3> &oldDim, double scalingStepSize,
                                               std::mt19937 &mt) const override;
};


#endif //RAMPACK_LINEARVOLUMESCALER_H
