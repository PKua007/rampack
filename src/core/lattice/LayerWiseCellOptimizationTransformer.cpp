//
// Created by pkua on 08.06.22.
//

#include "LayerWiseCellOptimizationTransformer.h"
#include "LatticeTraits.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/Packing.h"


LayerWiseCellOptimizationTransformer::LayerWiseCellOptimizationTransformer(LatticeTraits::Axis layerAxis,
                                                                           double spacing)
        : layerAxis{layerAxis}, spacing{spacing}
{

}

void LayerWiseCellOptimizationTransformer::transform(Lattice &lattice, const ShapeTraits &shapeTraits) const {
    const auto &interaction = shapeTraits.getInteraction();

    TransformerValidateMsg(interaction.hasHardPart(),
                           "Interaction must have hard component to perform layerwise optimization");
    TransformerValidateMsg(lattice.isRegular(), "Lattice must be regular to perform layerwise optimization");
    TransformerValidateMsg(lattice.isNormalized(),
                           "Relative coordinates in unit cell must be in range [0, 1) to perform layerwise "
                           "optimization");

    auto layerAssociation = LatticeTraits::getLayerAssociation(lattice.getUnitCell(), this->layerAxis);
    TransformerValidateMsg(!layerAssociation.empty(), "Lattice is empty; cannot perform layerwise optimization");

    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing testPacking(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), interaction);
    TransformerValidateMsg(!testPacking.countTotalOverlaps(interaction),
                           "Overlaps are present at the beginning of layerwise optimization");

    this->optimizeLayers(lattice, layerAssociation, testPacking, interaction);
    this->optimizeCell(lattice, testPacking, interaction);
    this->introduceSpacing(lattice, layerAssociation);
    this->centerShapesInCell(lattice.modifyUnitCellMolecules());

    lattice.normalize();
}

bool LayerWiseCellOptimizationTransformer::areShapesOverlapping(const TriclinicBox &box,
                                                                const std::vector<Shape> &shapes,
                                                                const std::array<std::size_t, 3> &latticeDim,
                                                                Packing &testPacking,
                                                                const Interaction &interaction) const
{
    Lattice testLattice(UnitCell(box, shapes), latticeDim);
    testLattice.normalize();
    testPacking.reset(testLattice.generateMolecules(), testLattice.getLatticeBox(), interaction);
    return testPacking.countTotalOverlaps(interaction);
}

void LayerWiseCellOptimizationTransformer::optimizeLayers(Lattice &lattice,
                                                          const LatticeTraits::LayerAssociation &layerAssociation,
                                                          Packing &testPacking, const Interaction &interaction) const
{
    const auto &cellBox = lattice.getCellBox();
    auto &cellShapes = lattice.modifyUnitCellMolecules();
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->layerAxis);

    // Optimize starting from the second layer, then third, etc. First layer remains untouched. So does the cell box.
    for (auto layer = std::next(layerAssociation.begin()); layer != layerAssociation.end(); layer++) {
        auto prevLayer = std::prev(layer);
        double begLayerCoord = prevLayer->first;
        double endLayerCoord = layer->first;

        constexpr double EPSILON = 1e-12;
        do {
            double midLayerCoord = (begLayerCoord + endLayerCoord) / 2;
            std::vector<Shape> midShapes = cellShapes;
            for (std::size_t i : layer->second) {
                auto &shape = midShapes[i];
                Vector<3> pos = shape.getPosition();
                pos[axisIdx] = midLayerCoord;
                shape.setPosition(pos);
            }

            if (this->areShapesOverlapping(cellBox, midShapes, lattice.getDimensions(), testPacking, interaction)) {
                begLayerCoord = midLayerCoord;
            } else {
                endLayerCoord = midLayerCoord;
                cellShapes = std::move(midShapes);
            }
        } while (std::abs(endLayerCoord - begLayerCoord) > EPSILON);

        // Move a bit optimized layer for greater numerical stability in cell optimization procedure - in essence
        // make sure that the margins is always between EPSILON and 2*EPSILON (which would be 0 to EPSILON without this
        // step - bisection may find the tangent position arbitrarily close "by accident")
        for (std::size_t i : layer->second) {
            auto &shape = cellShapes[i];
            Vector<3> pos = shape.getPosition();
            pos[axisIdx] += EPSILON;
            shape.setPosition(pos);
        }
    }
}

auto LayerWiseCellOptimizationTransformer::rescaleCell(const TriclinicBox &oldBox, const std::vector<Shape> &oldShapes,
                                                       double factor) const
{
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->layerAxis);
    std::array<Vector<3>, 3> newCellSides = oldBox.getSides();
    std::vector<Shape> newShapes = oldShapes;
    newCellSides[axisIdx] *= factor;
    for (auto &shape : newShapes) {
        Vector<3> pos = shape.getPosition();
        pos[axisIdx] /= factor;
        shape.setPosition(pos);
    }
    return std::make_pair(TriclinicBox(newCellSides), newShapes);
}

void LayerWiseCellOptimizationTransformer::optimizeCell(Lattice &lattice, Packing &testPacking,
                                                        const Interaction &interaction) const
{
    double range = testPacking.getTotalRangeRadius();       // testPacking already contains shapes, so this is working
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->layerAxis);
    auto &cellBox = lattice.modifyCellBox();
    auto &cellShapes = lattice.modifyUnitCellMolecules();

    const auto &boxHeights = lattice.getLatticeBox().getHeights();
    constexpr double FACTOR_EPSILON = 1 + 1e-12;
    // Verify initial dimensions whether they are large enough
    // range (plus epsilon), since we don't want self intersections through PBC
    TransformerValidateMsg(std::all_of(boxHeights.begin(), boxHeights.end(),
                                       [range](double d) { return d > FACTOR_EPSILON * range; }),
                           "Layerwise optimization: maximally shrunk cell (without PBC image self-overlaps) is too "
                           "large. Use more lattice cells.");

    // Bisectively move "upper" face of the cell until the shapes are tangent. Absolute shape coordinates should not
    // change in the process, so they relative ones are rescaled appropriately
    auto initialShapes = cellShapes;
    auto initialCellBox = cellBox;
    double begFactor = range * FACTOR_EPSILON / boxHeights[axisIdx];  // Smallest scaling without self-overlap
    double endFactor = 1;
    auto [begBox, begShapes] = this->rescaleCell(initialCellBox, initialShapes, begFactor);
    TransformerValidateMsg(this->areShapesOverlapping(begBox, begShapes, lattice.getDimensions(), testPacking,
                                                      interaction),
                           "Layerwise optimization: maximally shrunk cell (without PBC image self-overlaps) is not "
                           "overlapping. Use more lattice cells.");

    constexpr double EPSILON = 1e-12;
    do {
        double midFactor = (begFactor + endFactor) / 2;
        auto [midBox, midShapes] = this->rescaleCell(initialCellBox, initialShapes, midFactor);

        if (this->areShapesOverlapping(midBox, midShapes, lattice.getDimensions(), testPacking, interaction)) {
            begFactor = midFactor;
        } else {
            endFactor = midFactor;
            cellShapes = std::move(midShapes);
            cellBox = midBox;
        }
    } while (std::abs(endFactor - begFactor) > EPSILON);
}

void
LayerWiseCellOptimizationTransformer::introduceSpacing(Lattice &lattice,
                                                       const LatticeTraits::LayerAssociation &layerAssociation) const
{
    auto &cellBox = lattice.modifyCellBox();
    auto &cellShapes = lattice.modifyUnitCellMolecules();
    const auto &cellHeights = cellBox.getHeights();
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->layerAxis);
    double cellFactor = (cellHeights[axisIdx] + static_cast<double>(layerAssociation.size())*this->spacing)
                        / cellHeights[axisIdx];
    TransformerValidateMsg(cellFactor > 0,
                           "Layerwise optimization: negative spacing is of too large magnitude; it would yield "
                           "negative cell dimensions");

    auto cellSides = cellBox.getSides();
    cellSides[axisIdx] *= cellFactor;
    cellBox = TriclinicBox(cellSides);

    for (std::size_t i{}; i < layerAssociation.size(); i++) {
        for (std::size_t shapeIdx : layerAssociation[i].second) {
            auto &shape = cellShapes[shapeIdx];
            Vector<3> pos = shape.getPosition();
            pos[axisIdx] = (pos[axisIdx] + static_cast<double>(i)*this->spacing/cellHeights[axisIdx]) / cellFactor;
            shape.setPosition(pos);
        }
    }
}

void LayerWiseCellOptimizationTransformer::centerShapesInCell(std::vector<Shape> &cellShapes) const {
    std::size_t axisIdx = LatticeTraits::axisToIndex(this->layerAxis);
    double cellMiddle = (cellShapes.front().getPosition()[axisIdx] + cellShapes.back().getPosition()[axisIdx]) / 2;
    for (auto &shape : cellShapes) {
        Vector<3> pos = shape.getPosition();
        pos[axisIdx] += (0.5 - cellMiddle);
        shape.setPosition(pos);
    }
}
