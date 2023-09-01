//
// Created by Piotr Kubala on 01/09/2023.
//

#ifndef RAMPACK_S221CORRELATION_H
#define RAMPACK_S221CORRELATION_H

#include "AxisCorrelation.h"


class S221Correlation : public AxisCorrelation {
protected:
    [[nodiscard]] double calculateForAxes(const Vector<3> &axis1, const Vector<3> &axis2,
                                          [[maybe_unused]] const Vector<3> &distanceVector) const override
    {
        return ((axis1 ^ axis2) * distanceVector.normalized()) * (axis1 * axis2);
    }

public:
    /**
     * @brief Construct the class for a given @a axis axis type.
     */
    explicit S221Correlation(ShapeGeometry::Axis axis) : AxisCorrelation(axis) { }

    /**
     * @brief Returns "S221" as the signature name.
     */
    [[nodiscard]] std::string getSignatureName() const override { return "S221"; }
};


#endif //RAMPACK_S221CORRELATION_H
