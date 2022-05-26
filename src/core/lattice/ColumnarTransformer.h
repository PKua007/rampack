//
// Created by pkua on 20.05.22.
//

#ifndef RAMPACK_COLUMNARTRANSFORMER_H
#define RAMPACK_COLUMNARTRANSFORMER_H

#include <random>
#include <vector>
#include <utility>

#include "LatticeTransformer.h"
#include "LatticeTraits.h"


/**
 * @brief Takes a regular lattice and performs random translation on whole columns of particles in the direction of the
 * column. Irregular lattices are not supported.
 */
class ColumnarTransformer : public LatticeTransformer {
private:
    using ColumnCoord = std::array<double, 2>;
    using ColumnIndices = std::vector<std::size_t>;
    using ColumnAssociation = std::vector<std::pair<ColumnCoord, ColumnIndices>>;

    LatticeTraits::Axis columnAxis{};
    mutable std::mt19937 rng;

    [[nodiscard]] ColumnAssociation getColumnAssociation(const UnitCell &cell) const;

public:
    /**
     * @brief Constructs the object.
     * @param columnAxis axis along which columns should be created (axis is parallel to a corresponding lattice box
     * side)
     * @param seed seed for RNG used to translate columns
     */
    ColumnarTransformer(LatticeTraits::Axis columnAxis, unsigned long seed)
            : columnAxis{columnAxis}, rng{seed}
    { }

    /**
     * @brief Performs "columnarization" of the given @a lattice.
     * @details Columns are identified automatically in all unit unit cells as molecules having the same values of two
     * relative coordinates (cell relative coordinates plus integer lattice cell indices) which are not @a columnAxis
     * from the constructor. This third relative coordinate is then translated by a random amount moving the whole
     * column. Coordinate normalization is performed if needed.
     * @param lattice lattice to be "columnarized". It has to be regular and normalized (see Lattice::normalize()).
     */
    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_COLUMNARTRANSFORMER_H
