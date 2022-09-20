//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_CORRELATIONFUNCTION_H
#define RAMPACK_CORRELATIONFUNCTION_H

#include "core/Packing.h"
#include "core/ShapeTraits.h"


/**
 * @brief Correlation function of two shapes.
 */
class CorrelationFunction {
public:
    virtual ~CorrelationFunction() = default;

    /**
     * @brief For two shapes @a shape1 and @a shape2 characterized by @a shapeTraits, it return value of the specific
     * correlation function.
     */
    [[nodiscard]] virtual double calculate(const Shape &shape1, const Shape &shape2,
                                           const ShapeTraits &shapeTraits) const = 0;

    /**
     * @brief Returns the (short) name of this correlation function used in filenames and reporting.
     */
    [[nodiscard]] virtual std::string getSignatureName() const = 0;
};


#endif //RAMPACK_CORRELATIONFUNCTION_H
