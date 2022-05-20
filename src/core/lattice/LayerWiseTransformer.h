//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_LAYERWISETRANSFORMER_H
#define RAMPACK_LAYERWISETRANSFORMER_H

#include "LatticeTransformer.h"
#include "LatticeTraits.h"


class LayerWiseTransformer : public LatticeTransformer {
private:
    LatticeTraits::Axis axis;

    static std::size_t LCM(std::size_t n1, std::size_t n2);

    [[nodiscard]] std::vector<std::pair<double, std::vector<std::size_t>>>
    getLayerAssiciation(const UnitCell &cell) const;

    void recalculateUnitCell(UnitCell &cell,
                             std::vector<std::pair<double, std::vector<std::size_t>>> &layerAssociation,
                             std::array<std::size_t, 3> &dim, std::size_t requestedNumOfLayers) const;

protected:
    [[nodiscard]] virtual Shape transformShape(const Shape &shape, std::size_t layerIndex) const = 0;
    [[nodiscard]] virtual std::size_t getRequestedNumOfLayers() const = 0;

public:
    explicit LayerWiseTransformer(LatticeTraits::Axis axis) : axis{axis} { }

    void transform(Lattice &lattice) const final;
};


#endif //RAMPACK_LAYERWISETRANSFORMER_H
