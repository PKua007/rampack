//
// Created by Piotr Kubala on 16/05/2024.
//

#include <numeric>

#include "ReplicatingTransformer.h"
#include "utils/Exceptions.h"


ReplicatingTransformer::ReplicatingTransformer(const std::array<std::size_t, 3> &numReplicas, bool asUnitCell)
    : numReplicas{numReplicas}, asUnitCell{asUnitCell}
{
    Expects(std::all_of(numReplicas.begin(), numReplicas.end(), [](std::size_t n) { return n > 0; }));
    Expects(std::any_of(numReplicas.begin(), numReplicas.end(), [](std::size_t n) { return n > 1; }));
}

void ReplicatingTransformer::transform(Lattice &lattice, [[maybe_unused]] const ShapeTraits &shapeTraits) const {
    if (this->asUnitCell) {
        this->transformAsUnitCell(lattice);
        return;
    }

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
    auto dimensions = lattice.getDimensions();
    std::transform(dimensions.begin(), dimensions.end(), this->numReplicas.begin(), dimensions.begin(),
                   std::multiplies{});
    UnitCell cell(lattice.getCellBox(), {});
    Lattice newLattice(cell, dimensions);

    for (std::size_t i{}; i < this->numReplicas[0]; i++)
        for (std::size_t j{}; j < this->numReplicas[1]; j++)
            for (std::size_t k{}; k < this->numReplicas[2]; k++)
                ReplicatingTransformer::replicateCells(lattice, newLattice, {i, j, k});

    lattice = std::move(newLattice);
}

void ReplicatingTransformer::transformAsUnitCell(Lattice &lattice) const {
    auto cellDim = lattice.getLatticeBox();

    auto shapes = lattice.generateMolecules();
    for (auto &shape : shapes)
        shape.setPosition(cellDim.absoluteToRelative(shape.getPosition()));

    Lattice newLattice(UnitCell(cellDim, std::move(shapes)), this->numReplicas);
    lattice = std::move(newLattice);
}

void ReplicatingTransformer::replicateCells(const Lattice &originalLattice, Lattice &newLattice,
                                            const std::array<std::size_t, 3> &replicaI)
{
    const auto &dimensions = originalLattice.getDimensions();
    for (std::size_t i{}; i < dimensions[0]; i++) {
        for (std::size_t j{}; j < dimensions[1]; j++) {
            for (std::size_t k{}; k < dimensions[2]; k++) {
                auto &molecules= newLattice.modifySpecificCellMolecules(i + replicaI[0]*dimensions[0],
                                                                        j + replicaI[1]*dimensions[1],
                                                                        k + replicaI[2]*dimensions[2]);
                molecules = originalLattice.getSpecificCellMolecules(i, j, k);
            }
        }
    }
}
