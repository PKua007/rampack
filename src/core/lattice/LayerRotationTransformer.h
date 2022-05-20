//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_LAYERROTATIONTRANSFORMER_H
#define RAMPACK_LAYERROTATIONTRANSFORMER_H

#include "LayerWiseTransformer.h"
#include "LatticeTraits.h"


class LayerRotationTransformer : public LayerWiseTransformer {
private:
    std::size_t rotationAxisIdx{};
    double rotationAngle{};
    bool isAlternating{};

protected:
    [[nodiscard]] Shape transformShape(const Shape &shape, std::size_t layerIdx) const override;
    [[nodiscard]] std::size_t getRequestedNumOfLayers() const override;

public:
    LayerRotationTransformer(LatticeTraits::Axis layerAxis, LatticeTraits::Axis rotationAxis, double rotationAngle,
                             bool isAlternating);
};


#endif //RAMPACK_LAYERROTATIONTRANSFORMER_H
