//
// Created by pkua on 20.05.22.
//

#include <array>
#include <algorithm>

#include "ColumnarTransformer.h"


void ColumnarTransformer::transform(Lattice &lattice) const {
    Expects(lattice.isRegular());

    const auto &unitCell = lattice.getSpecificCell(0, 0, 0);
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
                std::uniform_real_distribution<double> dist;
                double shift = dist(this->mt);
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

std::vector<std::pair<std::array<double, 2>, std::vector<std::size_t>>>
ColumnarTransformer::getColumnAssociation(const UnitCell &cell) const
{
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->columnAxis);
    std::size_t idx1 = (axisIdx + 1) % 3;
    std::size_t idx2 = (axisIdx + 2) % 3;
    std::vector<std::pair<std::array<double, 2>, std::vector<std::size_t>>> columnsAssociation;

    for (std::size_t i{}; i < cell.size(); i++) {
        const auto &shape = cell[i];
        const auto &pos = shape.getPosition();
        std::array<double, 2> coord{pos[idx1], pos[idx2]};

        auto coordFinder = [coord](const auto &bin) {
            constexpr double EPSILON = 1e-10;
            return std::abs(bin.first[0] - coord[0]) < EPSILON && std::abs(bin.first[1] - coord[1]) < EPSILON;
        };
        auto it = std::find_if(columnsAssociation.begin(), columnsAssociation.end(), coordFinder);
        if (it == columnsAssociation.end()) {
            columnsAssociation.emplace_back(coord, std::vector<std::size_t>{});
            it = columnsAssociation.end() - 1;
        }
        it->second.push_back(i);
    }
    return columnsAssociation;
}
