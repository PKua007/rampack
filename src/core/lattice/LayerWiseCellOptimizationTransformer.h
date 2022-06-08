//
// Created by pkua on 08.06.22.
//

#ifndef RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H
#define RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H

#include "LatticeTransformer.h"
#include "LatticeTraits.h"


class LayerWiseCellOptimizationTransformer : public LatticeTransformer {
private:
    std::size_t axisIdx{};

public:
    explicit LayerWiseCellOptimizationTransformer(LatticeTraits::Axis layerAxis)
            : axisIdx{LatticeTraits::axisToIndex(layerAxis)}
    { }

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H
