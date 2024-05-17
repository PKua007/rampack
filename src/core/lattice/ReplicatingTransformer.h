//
// Created by Piotr Kubala on 16/05/2024.
//

#ifndef RAMPACK_REPLICATINGTRANSFORMER_H
#define RAMPACK_REPLICATINGTRANSFORMER_H

#include <array>

#include "LatticeTransformer.h"


class ReplicatingTransformer : public LatticeTransformer {
private:
    std::array<std::size_t, 3> numReplicas{};

    void transformRegular(Lattice &lattice) const;
    void transformIrregular(Lattice &lattice) const;

public:
    explicit ReplicatingTransformer(const std::array<std::size_t, 3> &numReplicas);

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_REPLICATINGTRANSFORMER_H
