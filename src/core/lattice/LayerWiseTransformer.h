//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_LAYERWISETRANSFORMER_H
#define RAMPACK_LAYERWISETRANSFORMER_H

#include "LatticeTransformer.h"
#include "LatticeTraits.h"


class LayerWiseTransformer : public LatticeTransformer {
private:
    using LayerIndices = std::vector<std::size_t>;
    using LayerAssociation = std::vector<std::pair<double, LayerIndices>>;

    LatticeTraits::Axis axis;

    static std::size_t LCM(std::size_t n1, std::size_t n2);

    [[nodiscard]] LayerAssociation getLayerAssociation(const UnitCell &cell) const;

    void recalculateUnitCell(UnitCell &cell, LayerAssociation &layerAssociation, std::array<std::size_t, 3> &latticeDim,
                             std::size_t requestedNumOfLayers) const;

protected:
    [[nodiscard]] virtual Shape transformShape(const Shape &shape, std::size_t layerIndex) const = 0;
    [[nodiscard]] virtual std::size_t getRequestedNumOfLayers() const = 0;

public:
    explicit LayerWiseTransformer(LatticeTraits::Axis axis) : axis{axis} { }

    void transform(Lattice &lattice) const final;
};


#endif //RAMPACK_LAYERWISETRANSFORMER_H
