//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_LOGVOLUMESCALER_H
#define RAMPACK_LOGVOLUMESCALER_H

#include "core/VolumeScaler.h"

/**
 * @brief A VolumeScaler sampling box side scaling factor from logarithmically sampled interval with endpoints given by
 * exp(+/- @a scalingStepSize).
 */
class LogVolumeScaler : public VolumeScaler {
private:
    ScalingDirection scalingDirection;
    bool scaleTogether = true;

    std::array<double, 3> sampleIndependentScalingFactors(double scalingStepSize, std::mt19937 &mt) const;

public:
    /**
     * @brief Construct the scaler with independent scaling directions given by @a scalingDirection.
     * @details If @a scaleTogether is true, all sides are perturbed at once. If false, only single independent set
     * of sides is perturbed.
     */
    explicit LogVolumeScaler(ScalingDirection scalingDirection, bool scaleTogether = true)
            : scalingDirection{scalingDirection}, scaleTogether{scaleTogether}
    { }

    std::array<double, 3> sampleScalingFactors(const std::array<double, 3> &oldDim, double scalingStepSize,
                                               std::mt19937 &mt) const override;
};


#endif //RAMPACK_LOGVOLUMESCALER_H
