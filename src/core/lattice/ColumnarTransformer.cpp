//
// Created by pkua on 20.05.22.
//

#include <array>
#include <algorithm>

#include "ColumnarTransformer.h"


void ColumnarTransformer::transform(Lattice &lattice) const {
    Expects(lattice.isRegular());
    Expects(lattice.isNormalized());

    const auto &unitCell = lattice.getUnitCell();
    auto columnAssociation = this->getColumnAssociation(unitCell);
    const auto &dim = lattice.getDimensions();

    std::size_t axisIdx = LatticeTraits::axisToIndex(this->columnAxis);
    std::size_t idx1 = (axisIdx + 1) % 3;
    std::size_t idx2 = (axisIdx + 2) % 3;

    std::array<std::size_t, 3> i{};
    i.fill(0);
    for (i[idx1] = 0; i[idx1] < dim[idx1]; i[idx1]++) {
        for (i[idx2] = 0; i[idx2] < dim[idx2]; i[idx2]++) {
            for (const auto &column : columnAssociation) {
                std::uniform_real_distribution<double> zeroToOne;
                double shift = zeroToOne(this->rng);
                for (i[axisIdx] = 0; i[axisIdx] < dim[axisIdx]; i[axisIdx]++) {
                    auto &cell = lattice.modifySpecificCellMolecules(i[0], i[1], i[2]);
                    for (auto cellIdx : column.second) {
                        auto &shape = cell[cellIdx];
                        auto pos = shape.getPosition();
                        pos[axisIdx] = pos[axisIdx] + shift;
                        while (pos[axisIdx] >= 1.0)
                            pos[axisIdx] -= 1.0;
                        while (pos[axisIdx] < 0.0)
                            pos[axisIdx] += 1.0;
                        shape.setPosition(pos);
                    }
                }
            }
        }
    }
}

ColumnarTransformer::ColumnAssociation ColumnarTransformer::getColumnAssociation(const UnitCell &cell) const {
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->columnAxis);
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
