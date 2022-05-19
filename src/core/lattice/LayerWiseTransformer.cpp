//
// Created by pkua on 19.05.22.
//

#include <algorithm>

#include "LayerWiseTransformer.h"
#include "utils/Assertions.h"


void LayerWiseTransformer::transform(Lattice &lattice) const {
    Expects(lattice.isRegular());

    auto cell = lattice.getCell(0, 0, 0);
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->axis);
    std::vector<std::pair<double, std::vector<std::size_t>>> layerAssociation;

    for (std::size_t i{}; i < cell.size(); i++) {
        const auto &shape = cell[i];
        double coord = shape.getPosition()[axisIdx];

        constexpr double EPSILON = 1e-10;
        auto coordFinder = [coord](const auto &bin) { return std::abs(bin.first - coord) < EPSILON; };
        auto it = std::find_if(layerAssociation.begin(), layerAssociation.end(), coordFinder);
        if (it == layerAssociation.end()) {
            layerAssociation.emplace_back(coord, std::vector<std::size_t>{});
            it = layerAssociation.end() - 1;
        }
        it->second.push_back(i);
    }

    if (layerAssociation.size() > 2)
        throw AssertionException("LayerWiseTransformer: more than 2 layers are unsupported");

    auto dim = lattice.getDimensions();
    if (layerAssociation.size() == 1) {
        if (dim[axisIdx] % 2 == 1)
            throw AssertionException("LayerWiseTransformer: there is an odd number of layers in the lattice");

        auto newCellShapeSides = cell.getCellShape().getSides();
        newCellShapeSides[axisIdx] *= 2.;
        TriclinicBox newCellShape(newCellShapeSides);

        std::vector<std::pair<double, std::vector<std::size_t>>> newLayerAssociation;
        newLayerAssociation.emplace_back(layerAssociation.front().first/2, layerAssociation.front().second);
        newLayerAssociation.emplace_back(layerAssociation.front().first/2 + 0.5, layerAssociation.front().second);
        for (auto &idx : newLayerAssociation.back().second)
            idx += cell.size();

        std::vector<Shape> newCellShapes;
        newCellShapes.reserve(2*cell.size());
        for (auto shape : cell) {
            auto pos = shape.getPosition();
            pos[axisIdx] /= 2;
            shape.setPosition(pos);
            newCellShapes.push_back(shape);
        }
        for (auto shape : cell) {
            auto pos = shape.getPosition();
            pos[axisIdx] /= 2;
            pos[axisIdx] += 2;
            shape.setPosition(pos);
            newCellShapes.push_back(shape);
        }

        layerAssociation = newLayerAssociation;
        cell = UnitCell(newCellShape, newCellShapes);
        dim[axisIdx] /= 2;
    }

    Assert(layerAssociation.size() == 2);
    for (auto idx : layerAssociation[0].second) {
        auto &cellShape = cell[idx];
        cellShape = this->transformShape(cellShape, true);
    }
    for (auto idx : layerAssociation[1].second) {
        auto &cellShape = cell[idx];
        cellShape = this->transformShape(cellShape, false);
    }

    Lattice newLattice(cell, dim);
    lattice = newLattice;
}
