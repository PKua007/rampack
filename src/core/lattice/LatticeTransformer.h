//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_LATTICETRANSFORMER_H
#define RAMPACK_LATTICETRANSFORMER_H

#include "Lattice.h"
#include "core/ShapeTraits.h"
#include "utils/Exceptions.h"


#define TransformerValidateMsg(cond, msg) EXCEPTIONS_BLOCK(                                                         \
    if (!(cond))                                                                                                    \
        throw TransformerException(msg);                                                                            \
)

/**
 * @brief Exception thrown when performing lattice transformations.
 */
class TransformerException : public ValidationException {
public:
    using ValidationException::ValidationException;
};


/**
 * @brief An interface representing a transformation performed on Lattice.
 */
class LatticeTransformer {
public:
    virtual ~LatticeTransformer() = default;

    /**
     * @brief Transforms a given @a lattice in a manner defined by implementing class.
     */
    virtual void transform(Lattice &lattice, const ShapeTraits &shapeTraits) const = 0;
};


#endif //RAMPACK_LATTICETRANSFORMER_H
