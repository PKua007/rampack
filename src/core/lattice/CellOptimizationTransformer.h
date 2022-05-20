//
// Created by pkua on 20.05.22.
//

#ifndef RAMPACK_CELLOPTIMIZATIONTRANSFORMER_H
#define RAMPACK_CELLOPTIMIZATIONTRANSFORMER_H

#include "LatticeTransformer.h"
#include "core/Interaction.h"


class CellOptimizationTransformer : public LatticeTransformer {
private:
    const Interaction &interaction;
    const std::string axisOrderString;

public:
    CellOptimizationTransformer(const Interaction &interaction, const std::string &axisOrderString);

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_CELLOPTIMIZATIONTRANSFORMER_H
