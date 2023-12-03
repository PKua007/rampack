//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_S110CORRELATION_H
#define RAMPACK_S110CORRELATION_H

#include "AxisCorrelation.h"


/**
 * @brief Calculates \f$ S_{110} \f$ correlation function for two shapes.
 * @details It is defined as \f$ \mathbf{\hat{a}}_i \cdot \mathbf{\hat{a}}_j \f$, where \f$ \mathbf{\hat{a}}_\alpha \f$
 * is either primary, secondary or auxiliary axis of \f$ \alpha^\mathrm{th} \f$ molecule.
 */
class S110Correlation : public AxisCorrelation {
protected:
    [[nodiscard]] double calculateForAxes(const Vector<3> &axis1, const Vector<3> &axis2,
                                          [[maybe_unused]] const Vector<3> &distanceVector) const override
    {
        return axis1 * axis2;
    }

public:
    /**
     * @brief Construct the class for a given @a axis axis type.
     */
    explicit S110Correlation(ShapeGeometry::Axis axis) : AxisCorrelation(axis) { }

    /**
     * @brief Returns "S110" as the signature name.
     */
    [[nodiscard]] std::string getSignatureName() const override { return "S110"; }
};


#endif //RAMPACK_S110CORRELATION_H
