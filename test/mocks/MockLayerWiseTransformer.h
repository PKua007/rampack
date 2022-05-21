//
// Created by pkua on 21.05.22.
//

#ifndef RAMPACK_MOCKLAYERWISETRANSFORMER_H
#define RAMPACK_MOCKLAYERWISETRANSFORMER_H

#include <catch2/trompeloeil.hpp>

#include "core/lattice/LayerWiseTransformer.h"


class MockLayerWiseTransformer : public LayerWiseTransformer {
protected:
    MAKE_CONST_MOCK2(transformShape, Shape(const Shape &shape, std::size_t layerIndex), override);
    MAKE_CONST_MOCK0(getRequestedNumOfLayers, std::size_t(), override);

public:
    explicit MockLayerWiseTransformer(LatticeTraits::Axis axis) : LayerWiseTransformer(axis) { }
};


#endif //RAMPACK_MOCKLAYERWISETRANSFORMER_H
