//
// Created by Piotr Kubala on 16/05/2024.
//

#ifndef RAMPACK_REPLICATINGTRANSFORMER_H
#define RAMPACK_REPLICATINGTRANSFORMER_H

#include <array>

#include "core/lattice/LatticeTransformer.h"


/**
 * @brief A LatticeTransformer which replicates the whole lattice a given number of times in each direction.
 */
class ReplicatingTransformer : public LatticeTransformer {
private:
    std::array<std::size_t, 3> numReplicas{};
    bool asUnitCell{};

    void transformRegular(Lattice &lattice) const;
    void transformIrregular(Lattice &lattice) const;
    void transformAsUnitCell(Lattice &lattice) const;

    static void replicateCells(const Lattice &originalLattice, Lattice &newLattice,
                               const std::array<std::size_t, 3> &replicaI);

public:
    /**
     * @brief Constructs the class.
     * @param numReplicas number of replicas to make. Subsequent indices of @a numReplicas correspond to, respectively,
     * x, y, and z direction.
     * @param asUnitCell if @a true, the original lattice is collapsed to a single unit cells and the replicas form
     * the new lattice. Otherwise, the original cells are preserved, but multiplied accordingly.
     */
    explicit ReplicatingTransformer(const std::array<std::size_t, 3> &numReplicas, bool asUnitCell = false);

    void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const override;
};


#endif //RAMPACK_REPLICATINGTRANSFORMER_H
