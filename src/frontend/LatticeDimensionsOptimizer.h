//
// Created by Piotr Kubala on 04/01/2023.
//

#ifndef RAMPACK_LATTICEDIMENSIONSOPTIMIZER_H
#define RAMPACK_LATTICEDIMENSIONSOPTIMIZER_H

#include <array>

#include "core/TriclinicBox.h"


class LatticeDimensionsOptimizer {
public:
    static std::array<std::size_t, 3> optimize(std::size_t numParticles, std::size_t cellSize,
                                               const TriclinicBox &requestedBox);
};


#endif //RAMPACK_LATTICEDIMENSIONSOPTIMIZER_H
