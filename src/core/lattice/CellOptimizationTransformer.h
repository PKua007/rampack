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
    std::array<double, 3> spacings{};

public:
    CellOptimizationTransformer(const Interaction &interaction, const std::string &axisOrderString, double spacing)
            : CellOptimizationTransformer(interaction, axisOrderString, {spacing, spacing, spacing})
    { }

    CellOptimizationTransformer(const Interaction &interaction, const std::string &axisOrderString,
                                const std::array<double, 3> &spacings);

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_CELLOPTIMIZATIONTRANSFORMER_H
