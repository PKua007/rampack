//
// Created by pkua on 08.06.22.
//

#ifndef RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H
#define RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H

#include "LatticeTransformer.h"
#include "LatticeTraits.h"
#include "core/Interaction.h"


class LayerWiseCellOptimizationTransformer : public LatticeTransformer {
private:
    LatticeTraits::Axis layerAxis;
    const Interaction &interaction;

public:
    explicit LayerWiseCellOptimizationTransformer(LatticeTraits::Axis layerAxis, const Interaction &interaction)
            : layerAxis{layerAxis}, interaction{interaction}
    { }

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_LAYERWISECELLOPTIMIZATIONTRANSFORMER_H
