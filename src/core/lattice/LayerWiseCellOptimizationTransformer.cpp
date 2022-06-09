//
// Created by pkua on 08.06.22.
//

#include "LayerWiseCellOptimizationTransformer.h"
#include "LatticeTraits.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/Packing.h"


void LayerWiseCellOptimizationTransformer::transform(Lattice &lattice) const {
    Expects(lattice.isRegular());
    Expects(lattice.isNormalized());

    auto layerAssociation = LatticeTraits::getLayerAssociation(lattice.getUnitCell(), this->layerAxis);
    Expects(!layerAssociation.empty());
    auto &cellBox = lattice.modifyCellBox();
    auto &cellShapes = lattice.modifyUnitCellMolecules();
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->layerAxis);

    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing testPacking(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), this->interaction);
    Expects(!testPacking.countTotalOverlaps(this->interaction));

    constexpr double EPSILON = 1e-12;

    for (auto it = std::next(layerAssociation.begin()); it != layerAssociation.end(); it++) {
        auto it0 = std::prev(it);
        double beg = it0->first;
        double end = it->first;
        std::vector<Shape> endShapes = cellShapes;

        do {
            double mid = (beg + end) / 2;
            std::vector<Shape> midShapes = cellShapes;
            for (std::size_t i : it->second) {
                auto &shape = midShapes[i];
                Vector<3> pos = shape.getPosition();
                pos[axisIdx] = mid;
                shape.setPosition(pos);
            }

            Lattice testLattice(UnitCell(cellBox, midShapes), lattice.getDimensions());
            testLattice.normalize();
            testPacking.reset(testLattice.generateMolecules(), testLattice.getLatticeBox(), this->interaction);
            if (testPacking.countTotalOverlaps(this->interaction)) {
                beg = mid;
            } else {
                end = mid;
                cellShapes = std::move(midShapes);
            }
        } while (std::abs(end - beg) > EPSILON);
    }

    lattice.normalize();
}
