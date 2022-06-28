//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_VOLUMESCALER_H
#define RAMPACK_VOLUMESCALER_H

#include <array>
#include <random>

/**
 * @brief A class used to sample volume scaling factor in a certain way/
 */
class VolumeScaler {
public:
    virtual ~VolumeScaler() = default;

    /**
     * @brief Given old dimensions of the box @a oldDim and @a scalingStepSize, the methods samples scaling factors
     * by which @a oldDim should by scaled according to class-specific distribution.
     * @details @a mt RNG generator is used to sample one of more random numbers needed.
     */
    virtual std::array<double, 3> sampleScalingFactors(const std::array<double, 3> &oldDim, double scalingStepSize,
                                                       std::mt19937 &mt) const = 0;
};


#endif //RAMPACK_VOLUMESCALER_H
