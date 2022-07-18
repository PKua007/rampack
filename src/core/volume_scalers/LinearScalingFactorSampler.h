//
// Created by pkua on 28.06.22.
//

#ifndef RAMPACK_LINEARSCALINGFACTORSAMPLER_H
#define RAMPACK_LINEARSCALINGFACTORSAMPLER_H

#include "ScalingFactorSampler.h"


/**
 * @brief Samples scaling factors in a linear way.
 */
class LinearScalingFactorSampler : public ScalingFactorSampler {
public:
    /**
     * @brief Samples factors linearly.
     * @details First, a random number from [@a -stepSize, @a stepSize] interval is drawn uniformly. Then, this random
     * number is added to each box height which should be scaled (as per @a shouldSample). Finally, factors are
     * calculated as ratios of new and old box heights.
     */
    [[nodiscard]] std::array<double, 3> sampleFactors(std::array<bool, 3> shouldSample,
                                                      const std::array<double, 3> &boxHeights, double stepSize,
                                                      std::mt19937 &mt) const override;
};


#endif //RAMPACK_LINEARSCALINGFACTORSAMPLER_H
