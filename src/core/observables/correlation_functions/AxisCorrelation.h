//
// Created by Piotr Kubala on 01/09/2023.
//

#ifndef RAMPACK_AXISCORRELATION_H
#define RAMPACK_AXISCORRELATION_H

#include "core/observables/CorrelationFunction.h"

/**
 * @brief Convenient wrapper for a correlation function, that uses a given type of shape axes (primary, secondary or
 * auxiliary) and distance vector (see PairConsumer::consumePair).
 */
class AxisCorrelation : public CorrelationFunction {
private:
    ShapeGeometry::Axis axis;

protected:
    /**
     * @brief Method which derived classes implement.
     * @details A given type of shape axis for both shape is passed (@a axis1, @a axis2) as well as @a distanceVector
     * for the shapes.
     */
    [[nodiscard]] virtual double calculateForAxes(const Vector<3> &axis1, const Vector<3> &axis2,
                                                  const Vector<3> &distanceVector) const = 0;

public:
    /**
     * @brief Construct the class for a given @a axis axis type.
     */
    explicit AxisCorrelation(ShapeGeometry::Axis axis) : axis{axis} { }

    [[nodiscard]] double calculate(const Shape &shape1, const Shape &shape2, const Vector<3> &distanceVector,
                                   const ShapeTraits &shapeTraits) const final;
};


#endif //RAMPACK_AXISCORRELATION_H
