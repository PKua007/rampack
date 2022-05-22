//
// Created by pkua on 19.05.22.
//

#include <algorithm>

#include "LayerWiseTransformer.h"
#include "utils/Assertions.h"


void LayerWiseTransformer::transform(Lattice &lattice) const {
    Expects(lattice.isRegular());

    auto cell = lattice.getSpecificCell(0, 0, 0);
    auto layerAssociation = this->getLayerAssociation(cell);
    std::size_t requestedNumOfLayers = this->getRequestedNumOfLayers();
    auto dim = lattice.getDimensions();

    this->recalculateUnitCell(cell, layerAssociation, dim, requestedNumOfLayers);
    Assert(layerAssociation.size() % requestedNumOfLayers == 0);
    std::size_t numOfLayers = layerAssociation.size();

    for (std::size_t layerIdx{}; layerIdx < numOfLayers; layerIdx++){
        const auto &layerShapes = layerAssociation[layerIdx].second;
        for (auto shapeIdx : layerShapes) {
            auto &cellShape = cell[shapeIdx];
            cellShape = this->transformShape(cellShape, layerIdx % requestedNumOfLayers);
        }
    }

    Lattice newLattice(cell, dim);
    lattice = newLattice;
}

void
LayerWiseTransformer::recalculateUnitCell(UnitCell &cell,
                                          std::vector<std::pair<double, std::vector<std::size_t>>> &layerAssociation,
                                          std::array<std::size_t, 3> &dim, std::size_t requestedNumOfLayers) const
{
    std::size_t numOfLayers = layerAssociation.size();
    std::size_t newNumOfLayers = LayerWiseTransformer::LCM(numOfLayers, requestedNumOfLayers);
    std::size_t factor = newNumOfLayers / numOfLayers;
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->axis);

    // Modify dimensions
    ExpectsMsg(dim[axisIdx] % factor == 0, "Lattice dimensions incompatible with the requested number of layers");
    dim[axisIdx] /= factor;

    // Modify layerAssociation
    std::vector<std::pair<double, std::vector<std::size_t>>> newLayerAssociation;
    newLayerAssociation.reserve(newNumOfLayers);
    std::size_t cellSize = cell.size();
    for (std::size_t i{}; i < factor; i++) {
        for (const auto &layer : layerAssociation) {
            double newCoord = layer.first;
            newCoord += static_cast<double>(i);
            newCoord += static_cast<double>(factor);
            auto newIdxs = layer.second;
            for (auto &newIdx : newIdxs)
                newIdx += i * cellSize;
            newLayerAssociation.emplace_back(newCoord, newIdxs);
        }
    }
    layerAssociation = newLayerAssociation;

    // Modify cell
    auto newCellShapeSides = cell.getBox().getSides();
    newCellShapeSides[axisIdx] *= static_cast<double>(factor);
    TriclinicBox newCellShape(newCellShapeSides);

    std::vector<Shape> newCellShapes;
    newCellShapes.reserve(factor*cellSize);
    for (std::size_t i{}; i < factor; i++) {
        for (auto shape : cell) {
            auto pos = shape.getPosition();
            pos[axisIdx] += static_cast<double>(i);
            pos[axisIdx] /= static_cast<double>(factor);
            shape.setPosition(pos);
            newCellShapes.push_back(shape);
        }
    }

    cell = UnitCell(newCellShape, newCellShapes);
}

std::vector<std::pair<double, std::vector<std::size_t>>>
LayerWiseTransformer::getLayerAssociation(const UnitCell &cell) const
{
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->axis);
    std::vector<std::pair<double, std::vector<std::size_t>>> layerAssociation;

    for (std::size_t i{}; i < cell.size(); i++) {
        const auto &shape = cell[i];
        double coord = shape.getPosition()[axisIdx];

        auto coordFinder = [coord](const auto &bin) {
            constexpr double EPSILON = 1e-10;
            return std::abs(bin.first - coord) < EPSILON;
        };
        auto it = std::find_if(layerAssociation.begin(), layerAssociation.end(), coordFinder);
        if (it == layerAssociation.end()) {
            layerAssociation.emplace_back(coord, std::vector<std::size_t>{});
            it = layerAssociation.end() - 1;
        }
        it->second.push_back(i);
    }
    return layerAssociation;
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
