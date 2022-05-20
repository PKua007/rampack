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
    LatticeTraits::Axis columnAxis{};
    mutable std::mt19937 mt;

    [[nodiscard]] std::vector<std::pair<std::array<double, 2>, std::vector<std::size_t>>>
    getColumnAssociation(const UnitCell &cell) const;

public:
    ColumnarTransformer(LatticeTraits::Axis columnAxis, unsigned long seed)
            : columnAxis{columnAxis}, mt{seed}
    { }

    void transform(Lattice &lattice) const override;
};


#endif //RAMPACK_COLUMNARTRANSFORMER_H
