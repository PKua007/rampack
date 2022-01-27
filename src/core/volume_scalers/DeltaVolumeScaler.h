//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_DELTAVOLUMESCALER_H
#define RAMPACK_DELTAVOLUMESCALER_H

#include "core/VolumeScaler.h"

/**
 * @brief A VolumeScaler sampling volume changes from a uniform interval with endpoints given by +/- @a scalingStepSize.
 */
class DeltaVolumeScaler : public VolumeScaler {
public:
    std::array<double, 3> sampleScalingFactors(const std::array<double, 3> &oldDim, double scalingStepSize,
                                               std::mt19937 &mt) const override;
};


#endif //RAMPACK_DELTAVOLUMESCALER_H
