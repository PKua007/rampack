//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_VOLUMESCALER_H
#define RAMPACK_VOLUMESCALER_H

#include <array>
#include <random>

class VolumeScaler {
public:
    enum class ScalingDirection {
        ISOTROPIC,
        ANISOTROPIC_X,
        ANISOTROPIC_Y,
        ANISOTROPIC_Z,
        ANISOTROPIC_XYZ
    };

    virtual std::array<double, 3> sampleScalingFactors(const std::array<double, 3> &oldDim, double scalingStepSize,
                                                       std::mt19937 &mt) const = 0;
};


#endif //RAMPACK_VOLUMESCALER_H
