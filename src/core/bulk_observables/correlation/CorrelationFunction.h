//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_CORRELATIONFUNCTION_H
#define RAMPACK_CORRELATIONFUNCTION_H

#include "core/Packing.h"
#include "core/ShapeTraits.h"


class CorrelationFunction {
public:
    virtual ~CorrelationFunction() = default;

    [[nodiscard]] virtual double calculate(const Shape &shape1, const Shape &shape2,
                                           const ShapeTraits &shapeTraits) const = 0;
};


#endif //RAMPACK_CORRELATIONFUNCTION_H
