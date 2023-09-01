//
// Created by Piotr Kubala on 20/04/2023.
//

#ifndef RAMPACK_AXESANGLE_H
#define RAMPACK_AXESANGLE_H


#include "core/observables/CorrelationFunction.h"


/**
 * @brief Angle between the axes of particles in degrees (the smaller one).
 */
class AxesAngle : public CorrelationFunction {
private:
    ShapeGeometry::Axis axis;

public:
    /**
     * @brief Construct the class for a given @a axis axis type.
     */
    explicit AxesAngle(ShapeGeometry::Axis axis) : axis{axis} { }

    [[nodiscard]] double calculate(const Shape &shape1, const Shape &shape2, const Vector<3> &distanceVector,
                                   const ShapeTraits &shapeTraits) const override;

    /**
     * @brief Returns "theta" as the signature name.
     */
    [[nodiscard]] std::string getSignatureName() const override { return "theta"; }
};


#endif //RAMPACK_AXESANGLE_H
