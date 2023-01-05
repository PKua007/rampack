//
// Created by Piotr Kubala on 04/01/2023.
//

#include <algorithm>

#include "LatticeDimensionsOptimizer.h"


std::array<std::size_t, 3> LatticeDimensionsOptimizer::optimize(std::size_t numParticles, std::size_t cellSize,
                                                                const TriclinicBox &requestedBox)
{
    std::size_t numAllCells;
    if (numParticles % cellSize == 0)
        numAllCells = numParticles / cellSize;
    else
        numAllCells = numParticles / cellSize + 1;

    auto heights = requestedBox.getHeights();
    double pseudoVolume = std::accumulate(heights.begin(), heights.end(), 1., std::multiplies<>{});
    double targetCellSize = std::cbrt(pseudoVolume / static_cast<double>(numAllCells));

    // Find the best integer number of cells
    std::array<std::size_t, 3> latticeDim{};
    auto dimCalculator = [targetCellSize](double height) {
        double bestNumCells = height/targetCellSize;
        return static_cast<std::size_t>(std::round(bestNumCells));
    };
    std::transform(heights.begin(), heights.end(), latticeDim.begin(), dimCalculator);

    // Increase number of cells along the longest side if there are too few cells to fit all particles
    while (std::accumulate(latticeDim.begin(), latticeDim.end(), 1ul, std::multiplies<>{}) < numAllCells) {
        auto maxDim = std::max_element(latticeDim.begin(), latticeDim.end());
        (*maxDim)++;
    }

    return latticeDim;
}
