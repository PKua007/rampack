//
// Created by Piotr Kubala on 23/03/2023.
//

#ifndef RAMPACK_SHAPEFUNCTION_H
#define RAMPACK_SHAPEFUNCTION_H

#include <string>

#include "core/Shape.h"
#include "core/ShapeTraits.h"


/**
 * @brief A single-value function computed for a given shape. It may be for example a give coordinate of a given shape
 * axis.
 */
class ShapeFunction {
public:
    virtual ~ShapeFunction() = default;

    /**
     * @brief Returns the function value for the given @a shape based on its @a traits ShapeTraits.
     */
    [[nodiscard]] virtual double calculate(const Shape &shape, const ShapeTraits &traits) const = 0;

    /**
     * @brief Returns the short name representing the function.
     */
    [[nodiscard]] virtual std::string getName() const = 0;
};


#endif //RAMPACK_SHAPEFUNCTION_H
