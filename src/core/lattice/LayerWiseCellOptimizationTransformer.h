//
// Created by pkua on 08.06.22.
//

#ifndef RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H
#define RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H

#include "LatticeTransformer.h"
#include "LatticeTraits.h"
#include "core/Interaction.h"
#include "core/Packing.h"


/**
 * @brief A Transformer which optimizes distances between all layers, together with cell dimension, in a given
 * direction.
 */
class LayerWiseCellOptimizationTransformer : public LatticeTransformer {
private:
    const Interaction &interaction;
    LatticeTraits::Axis layerAxis;
    double spacing;

    [[nodiscard]] bool areShapesOverlapping(const TriclinicBox &box, const std::vector<Shape> &shapes,
                                            const std::array<std::size_t, 3> &latticeDim, Packing &testPacking) const;
    void optimizeLayers(Lattice &lattice, const LatticeTraits::LayerAssociation &layerAssociation,
                        Packing &testPacking) const;
    [[nodiscard]] auto rescaleCell(const TriclinicBox &oldBox, const std::vector<Shape> &oldShapes,
                                   double factor) const;
    void optimizeCell(Lattice &lattice, Packing &testPacking) const;
    void introduceSpacing(Lattice &lattice, const LatticeTraits::LayerAssociation &layerAssociation) const;
    void centerShapesInCell(std::vector<Shape> &cellShapes) const;

public:
    /**
     * @brief Prepares the class for a given @a interaction, which will optimize layers and cell dimension in the
     * direction given by @a layerAxis and then space all layers by @a spacing (as measured by cell box heights).
     */
    explicit LayerWiseCellOptimizationTransformer(const Interaction &interaction, LatticeTraits::Axis layerAxis,
                                                  double spacing);

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H