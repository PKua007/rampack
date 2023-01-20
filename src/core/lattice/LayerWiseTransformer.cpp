//
// Created by pkua on 19.05.22.
//

#include <algorithm>

#include "LayerWiseTransformer.h"
#include "utils/Assertions.h"


void LayerWiseTransformer::transform(Lattice &lattice, [[maybe_unused]] const ShapeTraits &shapeTraits) const {
    Expects(lattice.isRegular());
    Expects(lattice.isNormalized());

    auto cell = lattice.getSpecificCell(0, 0, 0);
    auto layerAssociation = LatticeTraits::getLayerAssociation(cell, this->axis);
    std::size_t requestedNumOfLayers = this->getRequestedNumOfLayers();
    auto dim = lattice.getDimensions();

    this->recalculateUnitCell(cell, layerAssociation, dim, requestedNumOfLayers);
    Assert(layerAssociation.size() % requestedNumOfLayers == 0);
    std::size_t numOfLayers = layerAssociation.size();

    for (std::size_t layerIdx{}; layerIdx < numOfLayers; layerIdx++) {
        const auto &layerShapes = layerAssociation[layerIdx].second;
        for (auto shapeIdx : layerShapes) {
            auto &cellShape = cell[shapeIdx];
            cellShape = this->transformShape(cellShape, layerIdx % requestedNumOfLayers);
        }
    }

    Lattice newLattice(cell, dim);
    lattice = newLattice;
}

void LayerWiseTransformer::recalculateUnitCell(UnitCell &cell, LatticeTraits::LayerAssociation &layerAssociation,
                                               std::array<std::size_t, 3> &latticeDim,
                                               std::size_t requestedNumOfLayers) const
{
    std::size_t numOfLayers = layerAssociation.size();
    // As many cells will be merged as is needed to contain requested number of layers preserving periodicity
    std::size_t newNumOfLayers = LayerWiseTransformer::LCM(numOfLayers, requestedNumOfLayers);
    std::size_t cellFactor = newNumOfLayers / numOfLayers;
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->axis);

    // Modify dimensions
    ExpectsMsg(latticeDim[axisIdx] % cellFactor == 0,
               "Lattice dimensions incompatible with the requested number of layers");
    latticeDim[axisIdx] /= cellFactor;

    // Modify layerAssociation - as many unit cells are joined, we have to replicate layers "cellFactor" times
    // The distance between adjacent layer coords will also be scaled by "cellFactor", because it is relative to the
    // cell size
    std::vector<std::pair<double, std::vector<std::size_t>>> newLayerAssociation;
    newLayerAssociation.reserve(newNumOfLayers);
    std::size_t numMoleculesInCell = cell.size();
    for (std::size_t i{}; i < cellFactor; i++) {
        for (const auto &layer : layerAssociation) {
            double newLayerCoord = layer.first;
            newLayerCoord += static_cast<double>(i);
            newLayerCoord += static_cast<double>(cellFactor);
            auto newMoleculeIdxs = layer.second;
            // Indices of replicated molecules are shifted according to replica number preserving the order
            for (auto &newIdx : newMoleculeIdxs)
                newIdx += i * numMoleculesInCell;
            newLayerAssociation.emplace_back(newLayerCoord, newMoleculeIdxs);
        }
    }
    layerAssociation = newLayerAssociation;

    // Modify cell - similarly as for layer association, molecules have to be replicated "cellFactor" times with
    // appropriate scaling of relative coordinates (cell gets bigger)
    auto newCellShapeSides = cell.getBox().getSides();
    newCellShapeSides[axisIdx] *= static_cast<double>(cellFactor);
    TriclinicBox newCellShape(newCellShapeSides);

    std::vector<Shape> newCellShapes;
    newCellShapes.reserve(cellFactor * numMoleculesInCell);
    for (std::size_t i{}; i < cellFactor; i++) {
        for (auto shape : cell) {
            auto pos = shape.getPosition();
            pos[axisIdx] += static_cast<double>(i);
            pos[axisIdx] /= static_cast<double>(cellFactor);
            shape.setPosition(pos);
            newCellShapes.push_back(shape);
        }
    }

    cell = UnitCell(newCellShape, newCellShapes);
}

std::size_t LayerWiseTransformer::LCM(std::size_t n1, std::size_t n2) {
    Expects(n1 > 0);
    Expects(n2 > 0);

    if (n1 < n2)
        std::swap(n1, n2);

    std::size_t n1_0 = n1;
    while (n1 % n2 != 0)
        n1 += n1_0;
    return n1;
}
