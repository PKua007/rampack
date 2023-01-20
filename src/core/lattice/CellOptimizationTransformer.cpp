//
// Created by pkua on 20.05.22.
//

#include <numeric>

#include "CellOptimizationTransformer.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/Packing.h"
#include "DistanceOptimizer.h"
#include "utils/Assertions.h"
#include "LatticeTraits.h"


void CellOptimizationTransformer::transform(Lattice &lattice, const ShapeTraits &shapeTraits) const {
    const auto &interaction = shapeTraits.getInteraction();

    Expects(interaction.hasHardPart());
    Expects(lattice.isNormalized());

    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing testPacking(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), interaction);

    auto oldBox = testPacking.getBox();
    DistanceOptimizer::shrinkPacking(testPacking, interaction, this->axisOrderString);
    auto newBox = testPacking.getBox();

    const auto &newHeights = newBox.getHeights();
    const auto &dim = lattice.getDimensions();
    for (std::size_t i{}; i < 3; i++)
        Expects(newHeights[i] + this->spacings[i] * static_cast<double>(dim[i]) > 0);
    auto newSides = newBox.getSides();
    for (std::size_t i{}; i < 3; i++) {
        double scaleFactor = (newHeights[i] + spacings[i] * static_cast<double>(dim[i])) / newHeights[i];
        newSides[i] *= scaleFactor;
    }
    newBox = TriclinicBox(newSides);

    auto cellTransform = newBox.getDimensions() * oldBox.getDimensions().inverse();
    lattice.modifyCellBox().transform(cellTransform);
}

CellOptimizationTransformer::CellOptimizationTransformer(const std::string &axisOrderString,
                                                         const std::array<double, 3> &spacings)
        : axisOrderString{axisOrderString}, spacings{spacings}
{
    // Validate axis order string - it will throw if incorrect
    static_cast<void>(LatticeTraits::parseAxisOrder(axisOrderString));
}
