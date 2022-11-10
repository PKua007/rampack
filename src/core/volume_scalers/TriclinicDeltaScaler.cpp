//
// Created by pkua on 22.02.2022.
//

#include "TriclinicDeltaScaler.h"

TriclinicBox TriclinicDeltaScaler::updateBox(const TriclinicBox &oldBox, std::mt19937 &mt) const
{
    auto dimensions = oldBox.getDimensions();
    std::uniform_real_distribution<double> stepDistribution(-this->stepSize, this->stepSize);

    if (this->scaleTogether) {
        for (std::size_t i{}; i < 3; i++)
            for (std::size_t j{}; j < 3; j++)
                dimensions(i, j) += stepDistribution(mt);
    } else {
        std::uniform_int_distribution<std::size_t> sideDistribution(0, 2);
        std::size_t side = sideDistribution(mt);

        for (std::size_t i{}; i < 3; i++)
            dimensions(i, side) += stepDistribution(mt);
    }

    return TriclinicBox(dimensions);
}
