//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_LOGVOLUMESCALER_H
#define RAMPACK_LOGVOLUMESCALER_H

#include "core/VolumeScaler.h"

class LogVolumeScaler : public VolumeScaler {
private:
    ScalingDirection scalingDirection;

public:
    explicit LogVolumeScaler(ScalingDirection scalingDirection) : scalingDirection{scalingDirection} { }

    std::array<double, 3> sampleScalingFactors(const std::array<double, 3> &oldDim, double scalingStepSize,
                                               std::mt19937 &mt) const override;
};


#endif //RAMPACK_LOGVOLUMESCALER_H
