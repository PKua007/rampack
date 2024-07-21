//
// Created by Piotr Kubala on 21/07/2024.
//

#include <random>

#include "DiscreteShapeParameterRandomizer.h"
#include "utils/Exceptions.h"


DiscreteShapeParameterRandomizer::DiscreteShapeParameterRandomizer(std::vector<std::string> values)
        : values{std::move(values)}
{
    Expects(this->values.size() >= 2);
}

DiscreteShapeParameterRandomizer::DiscreteShapeParameterRandomizer(std::initializer_list<std::string> values)
        : values{values}
{
    Expects(this->values.size() >= 2);
}

std::string DiscreteShapeParameterRandomizer::randomize([[maybe_unused]] const std::string &oldValue,
                                                        std::mt19937 &mt) const
{
    std::uniform_int_distribution<std::size_t> indexDistribution(0, this->values.size() - 1);
    std::size_t index = indexDistribution(mt);
    return this->values[index];
}
