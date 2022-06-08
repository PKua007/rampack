//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_LAYERWISETRANSFORMER_H
#define RAMPACK_LAYERWISETRANSFORMER_H

#include "LatticeTransformer.h"
#include "LatticeTraits.h"


/**
 * @brief LatticeTransformer which performs implementation specific periodic set of modifications of whole layers in the
 * lattice.
 * @details Number of periodic layers and specific modifications applied to layers is specified by implementing methods
 * transformShape() and getRequestedNumOfLayers().
 */
class LayerWiseTransformer : public LatticeTransformer {
private:

    LatticeTraits::Axis axis;

    static std::size_t LCM(std::size_t n1, std::size_t n2);

    void recalculateUnitCell(UnitCell &cell, LatticeTraits::LayerAssociation &layerAssociation,
                             std::array<std::size_t, 3> &latticeDim, std::size_t requestedNumOfLayers) const;

protected:
    /**
     * @brief Transforms @a shape in an implementation specific manner, however given by @a layerIndex.
     * @details @a layerIndex always will always be in range [0, getRequestedNumOfLayers() - 1].
     */
    [[nodiscard]] virtual Shape transformShape(const Shape &shape, std::size_t layerIndex) const = 0;

    /**
     * @brief Returns requested number of layers.
     */
    [[nodiscard]] virtual std::size_t getRequestedNumOfLayers() const = 0;

public:
    /**
     * @brief Constructs the transformer object, which will perform layer transformation with layers along a given
     * @a axis.
     */
    explicit LayerWiseTransformer(LatticeTraits::Axis axis) : axis{axis} { }

    /**
     * @brief Transforms the given @a lattice.
     * @details First, layers are identified in a unit cell - molecules belong to the same layer if @a axis (from the
     * constructor) elements of their relative cell coordinates are the same. Then, if getRequestedNumOfLayers() gives
     * a number of layers which divides without remainder the number of layers found in the unit cell, the modifications
     * given by transformShape() are performed on each layer. However, if this condition is not met, as many adjacent
     * unit cells are joined into a single unit cell to make up for it. An exception will be thrown, if a number of
     * joint unit cells is incommensurate with the lattice size. The procedure is then carried on as previously
     * described.
     * @param lattice lattice to be transformed. Is has to be both regular and normalized. The resulting lattice remains
     * regular, but may no longer be normalized if transformShape() places the shape outside of a unit cell
     */
    void transform(Lattice &lattice) const final;
};


#endif //RAMPACK_LAYERWISETRANSFORMER_H
