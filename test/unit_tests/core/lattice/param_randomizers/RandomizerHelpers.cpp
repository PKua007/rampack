//
// Created by Piotr Kubala on 22/07/2024.
//

#include <algorithm>

#include "RandomizerHelpers.h"
#include "utils/Quantity.h"
#include "utils/Exceptions.h"


RandomizerStatistics::RandomizerStatistics(const ShapeParameterRandomizer &randomizer, std::size_t numSamples) {
    Expects(numSamples >= 2);

    std::mt19937 mt; // NOLINT(*-msc51-cpp)

    this->samples.reserve(numSamples);
    for (std::size_t i{}; i < this->samples.capacity(); i++)
        this->samples.push_back(std::stod(randomizer.randomize("", mt)));

    Quantity meanQuantity;
    meanQuantity.calculateFromSamples(this->samples);
    this->min = *std::min_element(this->samples.begin(), this->samples.end());
    this->max = *std::max_element(this->samples.begin(), this->samples.end());
    this->mean = meanQuantity.value;
    this->stddev = meanQuantity.error * std::sqrt(samples.size());     // Recalculate sample error from mean error
}

std::vector<std::string> sample_randomizer(const ShapeParameterRandomizer &randomizer, std::size_t numSamples) {
    std::mt19937 mt; // NOLINT(*-msc51-cpp)
    std::vector<std::string> samples;
    samples.reserve(numSamples);
    for (std::size_t i{}; i < samples.capacity(); i++)
        samples.push_back(randomizer.randomize("", mt));

    return samples;
}
