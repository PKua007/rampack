//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_LATTICETRANSFORMER_H
#define RAMPACK_LATTICETRANSFORMER_H

#include "Lattice.h"


class LatticeTransformer {
public:
    virtual ~LatticeTransformer() = default;

    virtual void transform(Lattice &lattice) const = 0;
};


#endif //RAMPACK_LATTICETRANSFORMER_H
