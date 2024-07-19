//
// Created by Piotr Kubala on 03/03/2024.
//

#ifndef RAMPACK_GAUSSIANSHAPEPARAMETERRANDOMIZER_H
#define RAMPACK_GAUSSIANSHAPEPARAMETERRANDOMIZER_H

#include <limits>

#include "ShapeParameterRandomizer.h"


/**
 * @brief ShapeParameterRandomizer randomizing floating point shape parameter according to a normal distribution.
 */
class GaussianShapeParameterRandomizer : public ShapeParameterRandomizer {
private:
    double mean{};
    double sigma{};
    double cutoff{};

public:
    /**
     * @brief Creates the randomizer for given parameters of the normal distribution.
     * @param mean mean of the normal distribution
     * @param sigma standard deviation of the normal distribution
     * @param cutoff optional cutoff of the distribution to limit its spread. More precisely, no random variables will
     * be sampled outside of the [@a mean - @a cutoff, @a mean + @a cutoff] range
     */
    GaussianShapeParameterRandomizer(double mean, double sigma,
                                     double cutoff = std::numeric_limits<double>::infinity());

    [[nodiscard]] std::string randomize(const std::string &oldValue, std::mt19937 &mt) const override;
};


#endif //RAMPACK_GAUSSIANSHAPEPARAMETERRANDOMIZER_H
