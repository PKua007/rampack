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
    virtual void calculate(const Shape &shape, const ShapeTraits &traits) = 0;

    [[nodiscard]] virtual std::string getPrimaryName() const = 0;

    [[nodiscard]] virtual std::vector<std::string> getNames() const = 0;

    [[nodiscard]] virtual std::vector<double> getValues() const = 0;
};


#endif //RAMPACK_SHAPEFUNCTION_H
