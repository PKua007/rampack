//
// Created by pkua on 27.06.22.
//

#ifndef RAMPACK_SCALINGFACTORSAMPLER_H
#define RAMPACK_SCALINGFACTORSAMPLER_H

#include <array>
#include <random>


/**
 * @brief An interface representing a specific way of sampling scaling factors.
 * @details They can be sampler for example linearly or logarithmically.
 */
class ScalingFactorSampler {
public:
    virtual ~ScalingFactorSampler() = default;

    /**
     * @brief Samples scaling factors in one or more scaling directions.
     * @param shouldSample @a true/false array describing which directions should be sampled (array slots are
     * subsequent x, y and z direction). All @a true direction should be sampled using the same random number, not
     * independently
     * @param boxHeights heights of the box (may be used for calculating factors)
     * @param stepSize maximal step size. Its meaning depends on an implementation
     * @param mt RNG used to sample factors
     * @return sampled factors (some may me equal 1 if the direction is not used for sampling)
     */
    [[nodiscard]] virtual std::array<double, 3> sampleFactors(std::array<bool, 3> shouldSample,
                                                              const std::array<double, 3> &boxHeights,
                                                              double stepSize, std::mt19937 &mt) const = 0;
};


#endif //RAMPACK_SCALINGFACTORSAMPLER_H
