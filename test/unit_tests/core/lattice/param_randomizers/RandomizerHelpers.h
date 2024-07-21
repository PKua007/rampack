//
// Created by Piotr Kubala on 22/07/2024.
//

#ifndef RAMPACK_RANDOMIZERHELPERS_H
#define RAMPACK_RANDOMIZERHELPERS_H

#include <vector>

#include "core/lattice/ShapeParameterRandomizer.h"


struct RandomizerStatistics {
    std::vector<double> samples;
    double min{};
    double max{};
    double mean{};
    double stddev{};

    explicit RandomizerStatistics(const ShapeParameterRandomizer &randomizer, std::size_t numSamples = 1000);
};

std::vector<std::string> sample_randomizer(const ShapeParameterRandomizer &randomizer, std::size_t numSamples = 1000);


#endif //RAMPACK_RANDOMIZERHELPERS_H
