//
// Created by Piotr Kubala on 16/05/2024.
//

#ifndef RAMPACK_REPLICATINGTRANSFORMER_H
#define RAMPACK_REPLICATINGTRANSFORMER_H

#include <array>

#include "LatticeTransformer.h"


/**
 * @brief A LatticeTransformer which replicates the whole lattice a given number of times in each direction.
 * @details If the lattice is regular, the number of cells in each direction is altered accordingly. The irregular
 * lattice is converted into a regular lattice by treating it as a single unit cell in the new replica lattice.
 */
class ReplicatingTransformer : public LatticeTransformer {
private:
    std::array<std::size_t, 3> numReplicas{};

    void transformRegular(Lattice &lattice) const;
    void transformIrregular(Lattice &lattice) const;

public:
    /**
     * @brief Constructs the class by creating a number of replicas given by subsequent indices of @a numReplicas
     * corresponding to, respectively, x, y, and z direction.
     */
    explicit ReplicatingTransformer(const std::array<std::size_t, 3> &numReplicas);

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_REPLICATINGTRANSFORMER_H
