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


class ColumnarTransformer : public LatticeTransformer {
private:
    using ColumnCoord = std::array<double, 2>;
    using ColumnIndices = std::vector<std::size_t>;
    using ColumnAssociation = std::vector<std::pair<ColumnCoord, ColumnIndices>>;

    LatticeTraits::Axis columnAxis{};
    mutable std::mt19937 rng;

    [[nodiscard]] ColumnAssociation getColumnAssociation(const UnitCell &cell) const;

public:
    ColumnarTransformer(LatticeTraits::Axis columnAxis, unsigned long seed)
            : columnAxis{columnAxis}, rng{seed}
    { }

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_COLUMNARTRANSFORMER_H
