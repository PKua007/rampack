//
// Created by Piotr Kubala on 23/03/2023.
//

#ifndef RAMPACK_SHAPEFUNCTION_H
#define RAMPACK_SHAPEFUNCTION_H

#include "core/Shape.h"
#include "core/ShapeTraits.h"


class ShapeFunction {
public:
    virtual ~ShapeFunction() = default;

    [[nodiscard]] virtual double calculate(const Shape &shape1, const Shape &shape2,
                                           const ShapeTraits &traits) const = 0;
};


#endif //RAMPACK_SHAPEFUNCTION_H
