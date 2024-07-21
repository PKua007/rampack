//
// Created by Piotr Kubala on 22/07/2024.
//

#include <random>
#include <sstream>
#include <iomanip>

#include "TriangularShapeParameterRandomizer.h"
#include "utils/Exceptions.h"


std::string TriangularShapeParameterRandomizer::randomize([[maybe_unused]] const std::string &oldValue,
                                                          std::mt19937 &mt) const
{
    std::piecewise_linear_distribution<double> distribution(this->intervals.begin(), this->intervals.end(),
                                                            this->weights.begin());
    double newValue = distribution(mt);
    std::ostringstream converter;
    converter << std::setprecision(std::numeric_limits<double>::max_digits10) << newValue;
    return converter.str();
}

TriangularShapeParameterRandomizer::TriangularShapeParameterRandomizer(double beg, double mid, double end)
        : intervals{beg, mid, end}, weights{0, 2./(end - beg), 0}
{
    Expects(beg < end);
    Expects(beg <= mid && mid <= end);
}
