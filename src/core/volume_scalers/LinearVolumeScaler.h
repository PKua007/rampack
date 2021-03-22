//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_LINEARVOLUMESCALER_H
#define RAMPACK_LINEARVOLUMESCALER_H

#include "core/VolumeScaler.h"

class LinearVolumeScaler : public VolumeScaler {
private:
    ScalingDirection scalingDirection{};

public:
    explicit LinearVolumeScaler(ScalingDirection scalingDirection) : scalingDirection{scalingDirection} { }

    std::array<double, 3> sampleScalingFactors(const std::array<double, 3> &oldDim, double scalingStepSize,
                                               std::mt19937 &mt) const override;
};


#endif //RAMPACK_LINEARVOLUMESCALER_H
