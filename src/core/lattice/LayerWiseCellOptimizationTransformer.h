//
// Created by pkua on 08.06.22.
//

#ifndef RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H
#define RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H

#include "LatticeTransformer.h"
#include "LatticeTraits.h"
#include "core/Interaction.h"
#include "core/Packing.h"


class LayerWiseCellOptimizationTransformer : public LatticeTransformer {
private:
    LatticeTraits::Axis layerAxis;
    const Interaction &interaction;

    void optimizeLayers(LatticeTraits::LayerAssociation &layerAssociation, const TriclinicBox &cellBox,
                        std::vector<Shape> &cellShapes, const std::array<std::size_t, 3> &dimensions,
                        Packing &testPacking) const;
    void optimizeCell(TriclinicBox &cellBox, std::vector<Shape> &cellShapes, const Lattice &lattice,
                      Packing &testPacking) const;
    void centerShapesInCell(std::vector<Shape> &cellShapes) const;
    bool areShapesOverlapping(const TriclinicBox &box, const std::vector<Shape> &shapes,
                              const std::array<std::size_t, 3> &latticeDim, Packing &testPacking) const;

public:
    explicit LayerWiseCellOptimizationTransformer(LatticeTraits::Axis layerAxis, const Interaction &interaction)
            : layerAxis{layerAxis}, interaction{interaction}
    { }

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H
