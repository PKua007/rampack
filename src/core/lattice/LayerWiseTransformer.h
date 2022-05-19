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

protected:
    [[nodiscard]] virtual Shape transformShape(const Shape &shape, bool isEven) const = 0;

public:
    explicit LayerWiseTransformer(LatticeTraits::Axis axis) : axis{axis} { }

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_LAYERWISETRANSFORMER_H
