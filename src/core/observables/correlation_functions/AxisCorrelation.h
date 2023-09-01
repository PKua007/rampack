//
// Created by Piotr Kubala on 01/09/2023.
//

#ifndef RAMPACK_AXISCORRELATION_H
#define RAMPACK_AXISCORRELATION_H

#include "core/observables/CorrelationFunction.h"


class AxisCorrelation : public CorrelationFunction {
private:
    ShapeGeometry::Axis axis;

protected:
    [[nodiscard]] virtual double calculateForAxes(const Vector<3> &axis1, const Vector<3> &axis2) const = 0;

public:
    /**
     * @brief Construct the class for a given @a axis axis type.
     */
    explicit AxisCorrelation(ShapeGeometry::Axis axis) : axis{axis} { }

    [[nodiscard]] double calculate(const Shape &shape1, const Shape &shape2,
                                   const ShapeTraits &shapeTraits) const final;
};


#endif //RAMPACK_AXISCORRELATION_H
