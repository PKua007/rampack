//
// Created by Piotr Kubala on 20/04/2023.
//

#ifndef RAMPACK_AXESANGLE_H
#define RAMPACK_AXESANGLE_H


#include "core/observables/CorrelationFunction.h"


class AxesAngle : public CorrelationFunction {
private:
    ShapeGeometry::Axis axis;

public:
    explicit AxesAngle(ShapeGeometry::Axis axis) : axis{axis} { }

    [[nodiscard]] double calculate(const Shape &shape1, const Shape &shape2,
                                   const ShapeTraits &shapeTraits) const override;

    [[nodiscard]] std::string getSignatureName() const override { return "theta"; }
};


#endif //RAMPACK_AXESANGLE_H
