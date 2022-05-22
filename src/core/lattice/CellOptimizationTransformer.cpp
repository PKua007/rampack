//
// Created by pkua on 20.05.22.
//

#include <algorithm>

#include "CellOptimizationTransformer.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/Packing.h"
#include "core/DistanceOptimizer.h"
#include "utils/Assertions.h"
#include "LatticeTraits.h"


void CellOptimizationTransformer::transform(Lattice &lattice) const {
    Expects(lattice.isRegular());

    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing testPacking(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), this->interaction);

    auto oldBox = testPacking.getBox();
    DistanceOptimizer::shrinkPacking(testPacking, this->interaction, this->axisOrderString);
    auto newBox = testPacking.getBox();

    auto cellTransform = newBox.getDimensions() * oldBox.getDimensions().inverse();
    lattice.modifyCellBox().transform(cellTransform);
}

CellOptimizationTransformer::CellOptimizationTransformer(const Interaction &interaction,
                                                         const std::string &axisOrderString)
        : interaction{interaction}, axisOrderString{axisOrderString}
{
    // Validate axis order string - it will throw if incorrect
    static_cast<void>(LatticeTraits::parseAxisOrder(axisOrderString));

    Expects(interaction.hasHardPart());
}
