//
// Created by pkua on 28.06.22.
//

#ifndef RAMPACK_LOGSCALINGFACTORSAMPLER_H
#define RAMPACK_LOGSCALINGFACTORSAMPLER_H

#include "ScalingFactorSampler.h"


/**
 * @brief Samples scaling factors in a logarithmic way.
 */
class LogScalingFactorSampler : public ScalingFactorSampler {
public:
    /**
     * @brief Samples factors logarithmically.
     * @details First, a random number @a w from [@a -stepSize, @a stepSize] interval is drawn uniformly. Then, factors
     * for each sampled direction are calculated as <em>exp(w)</em>.
     */
    [[nodiscard]] std::array<double, 3> sampleFactors(std::array<bool, 3> shouldSample,
                                                      const std::array<double, 3> &boxHeights, double stepSize,
                                                      std::mt19937 &mt) const override;

};


#endif //RAMPACK_LOGSCALINGFACTORSAMPLER_H
