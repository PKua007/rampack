//
// Created by pkua on 20.05.22.
//

#include <array>
#include <algorithm>

#include "ColumnarTransformer.h"
#include "LatticeTraits.h"
#include "core/ShapeTraits.h"


void ColumnarTransformer::transform(Lattice &lattice, [[maybe_unused]] const ShapeTraits &shapeTraits) const {
    TransformerValidateMsg(lattice.isRegular(), "Columnar transformation can only be applied to a regular lattice");
    TransformerValidateMsg(lattice.isNormalized(),
                           "Relative coordinates in unit cell must be in range [0, 1) to perform distance "
                           "optimization");

    const auto &unitCell = lattice.getUnitCell();
    auto columnAssociation = LatticeTraits::getColumnAssociation(unitCell, this->columnAxis);
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
