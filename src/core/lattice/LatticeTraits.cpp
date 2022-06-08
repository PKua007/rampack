//
// Created by pkua on 19.05.22.
//

#include <algorithm>

#include "LatticeTraits.h"


std::array<std::size_t, 3> LatticeTraits::parseAxisOrder(const std::string &axisOrderString) {
    if (axisOrderString == "xyz")
        return {0, 1, 2};
    else if (axisOrderString == "xzy")
        return {0, 2, 1};
    else if (axisOrderString == "yxz")
        return {1, 0, 2};
    else if (axisOrderString == "yzx")
        return {1, 2, 0};
    else if (axisOrderString == "zxy")
        return {2, 0, 1};
    else if (axisOrderString == "zyx")
        return {2, 1, 0};
    else
        throw AxisOrderParseException("Malformed axis order");
}

std::size_t LatticeTraits::axisToIndex(LatticeTraits::Axis axis) {
    return static_cast<std::size_t>(axis);
}

LatticeTraits::LayerAssociation LatticeTraits::getLayerAssociation(const UnitCell &cell,
                                                                   LatticeTraits::Axis layerAxis)
{
    std::size_t axisIdx = LatticeTraits::axisToIndex(layerAxis);
    LayerAssociation layerAssociation;

    for (std::size_t i{}; i < cell.size(); i++) {
        const auto &shape = cell[i];
        double axisPosElem = shape.getPosition()[axisIdx];

        auto layerCoordFinder = [axisPosElem](const auto &bin) {
            constexpr double EPSILON = 1e-10;
            return std::abs(bin.first - axisPosElem) < EPSILON;
        };
        auto it = std::find_if(layerAssociation.begin(), layerAssociation.end(), layerCoordFinder);
        if (it == layerAssociation.end()) {
            layerAssociation.emplace_back(axisPosElem, LayerIndices{});
            it = layerAssociation.end() - 1;
        }
        it->second.push_back(i);
    }
    return layerAssociation;
}

LatticeTraits::ColumnAssociation LatticeTraits::getColumnAssociation(const UnitCell &cell,
                                                                     LatticeTraits::Axis columnAxis)
{
    std::size_t axisIdx = LatticeTraits::axisToIndex(columnAxis);
    std::size_t idx1 = (axisIdx + 1) % 3;
    std::size_t idx2 = (axisIdx + 2) % 3;
    ColumnAssociation columnsAssociation;

    for (std::size_t shapeIdx{}; shapeIdx < cell.size(); shapeIdx++) {
        const auto &shape = cell[shapeIdx];
        const auto &pos = shape.getPosition();
        ColumnCoord columnCoord{pos[idx1], pos[idx2]};

        auto columnCoordFinder = [columnCoord](const auto &bin) {
            constexpr double EPSILON = 1e-10;
            return std::abs(bin.first[0]-columnCoord[0]) < EPSILON && std::abs(bin.first[1]-columnCoord[1]) < EPSILON;
        };
        auto it = std::find_if(columnsAssociation.begin(), columnsAssociation.end(), columnCoordFinder);
        if (it == columnsAssociation.end()) {
            columnsAssociation.emplace_back(columnCoord, ColumnIndices{});
            it = columnsAssociation.end() - 1;
        }
        it->second.push_back(shapeIdx);
    }
    return columnsAssociation;
}
