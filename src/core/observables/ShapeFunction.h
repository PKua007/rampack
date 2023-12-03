//
// Created by Piotr Kubala on 23/03/2023.
//

#ifndef RAMPACK_SHAPEFUNCTION_H
#define RAMPACK_SHAPEFUNCTION_H

#include <string>

#include "core/Shape.h"
#include "core/ShapeTraits.h"


/**
 * @brief A single or multi-valued function computed for a given shape. It may be for example a give coordinate of a
 * given shape axis.
 * @details Each function has the primary name (getPrimaryName()) describing all components as well as names of the
 * particular components (getNames()). For example, the primary name can be `axis` and component names can by `x`, `y`
 * and `z`, referring to subsequent coordinates. If the function is single-valued, the name of the only component should
 * be the same as the primary name.
 */
class ShapeFunction {
public:
    virtual ~ShapeFunction() = default;

    /**
     * @brief Calculates the function values for the given @a shape based on its @a traits ShapeTraits.
     * @details The values can be obtained using getValues().
     */
    virtual void calculate(const Shape &shape, const ShapeTraits &traits) = 0;

    /**
     * @brief Returns the primary name of the function, describing all components.
     */
    [[nodiscard]] virtual std::string getPrimaryName() const = 0;

    /**
     * @brief Returns vector of names of subsequent functions values.
     */
    [[nodiscard]] virtual std::vector<std::string> getNames() const = 0;

    /**
     * @brief Returns the vector of function values calculates in the last calculate() invocation.
     */
    [[nodiscard]] virtual std::vector<double> getValues() const = 0;
};


#endif //RAMPACK_SHAPEFUNCTION_H
