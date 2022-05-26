//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_LATTICETRANSFORMER_H
#define RAMPACK_LATTICETRANSFORMER_H

#include "Lattice.h"


/**
 * @brief An interface representing a transformation performed on Lattice.
 */
class LatticeTransformer {
public:
    virtual ~LatticeTransformer() = default;

    /**
     * @brief Transforms a given @a lattice in a manner defined by implementing class.
     */
    virtual void transform(Lattice &lattice) const = 0;
};


#endif //RAMPACK_LATTICETRANSFORMER_H
