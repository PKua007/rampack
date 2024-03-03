//
// Created by Piotr Kubala on 03/03/2024.
//

#include <sstream>
#include <iomanip>

#include "GaussianShapeParameterRandomizer.h"
#include "utils/Exceptions.h"


GaussianShapeParameterRandomizer::GaussianShapeParameterRandomizer(double mean, double sigma, double cutoff)
    : mean{mean}, sigma{sigma}, cutoff{cutoff}
{
    Expects(sigma > 0);
    Expects(cutoff > 0);
}

std::string GaussianShapeParameterRandomizer::randomize([[maybe_unused]] const std::string &oldValue,
                                                        std::mt19937 &mt) const
{
    std::normal_distribution<double> gaussian(this->mean, this->sigma);
    double newValue;
    do {
        newValue = gaussian(mt);
    } while (std::abs(newValue - this->mean) > this->cutoff);

    std::ostringstream converter;
    converter << std::setprecision(std::numeric_limits<double>::max_digits10) << newValue;
    return converter.str();
}
