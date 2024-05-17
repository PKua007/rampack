//
// Created by Piotr Kubala on 16/05/2024.
//

#include <numeric>

#include "ReplicatingTransformer.h"
#include "utils/Exceptions.h"


ReplicatingTransformer::ReplicatingTransformer(const std::array<std::size_t, 3> &numReplicas) : numReplicas{numReplicas}
{
    Expects(std::all_of(numReplicas.begin(), numReplicas.end(), [](std::size_t n) { return n > 0; }));
    Expects(std::any_of(numReplicas.begin(), numReplicas.end(), [](std::size_t n) { return n > 1; }));
}

void ReplicatingTransformer::transform(Lattice &lattice, [[maybe_unused]] const ShapeTraits &shapeTraits) const {
    if (lattice.isRegular())
        this->transformRegular(lattice);
    else
        this->transformIrregular(lattice);
}

void ReplicatingTransformer::transformRegular(Lattice &lattice) const {
    auto dimensions = lattice.getDimensions();
    std::transform(dimensions.begin(), dimensions.end(), this->numReplicas.begin(), dimensions.begin(),
                   std::multiplies{});
    lattice.changeRegularDimensions(dimensions);
}

void ReplicatingTransformer::transformIrregular(Lattice &lattice) const {
    auto cellDim = lattice.getLatticeBox();

    auto shapes = lattice.generateMolecules();
    for (auto &shape : shapes)
        shape.setPosition(cellDim.absoluteToRelative(shape.getPosition()));

    Lattice newLattice(UnitCell(cellDim, std::move(shapes)), this->numReplicas);
    lattice = std::move(newLattice);
}
