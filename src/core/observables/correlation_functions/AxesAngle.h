//
// Created by Piotr Kubala on 20/04/2023.
//

#ifndef RAMPACK_AXESANGLE_H
#define RAMPACK_AXESANGLE_H


#include "AxisCorrelation.h"


/**
 * @brief Angle between the axes of particles in degrees (the smaller one).
 */
class AxesAngle : public AxisCorrelation {
protected:
    [[nodiscard]] double calculateForAxes(const Vector<3> &axis1, const Vector<3> &axis2,
                                          [[maybe_unused]] const Vector<3> &distanceVector) const override
    {
        return 180 * std::acos(std::abs(axis1 * axis2)) / M_PI;
    }

public:
    /**
     * @brief Construct the class for a given @a axis axis type.
     */
    explicit AxesAngle(ShapeGeometry::Axis axis) : AxisCorrelation(axis) { }

    /**
     * @brief Returns "theta" as the signature name.
     */
    [[nodiscard]] std::string getSignatureName() const override { return "theta"; }
};


#endif //RAMPACK_AXESANGLE_H
