//
// Created by Piotr Kubala on 01/09/2023.
//

#ifndef RAMPACK_S220CORRELATION_H
#define RAMPACK_S220CORRELATION_H

#include "AxisCorrelation.h"


/**
 * @brief Calculates \f$ S_{220} \f$ correlation function for two shapes.
 * @details It is defined as \f$ 3/2 (\mathbf{\hat{a}}_i \cdot \mathbf{\hat{a}}_j - 1/3)\f$, where
 * \f$ \mathbf{\hat{a}}_\alpha \f$ is either primary, secondary or auxiliary axis of \f$ \alpha^\mathrm{th} \f$
 * molecule.
 */
class S220Correlation : public AxisCorrelation {
protected:
    [[nodiscard]] double calculateForAxes(const Vector<3> &axis1, const Vector<3> &axis2) const override {
        return 1.5 * (axis1 * axis2 - 1./3);
    }

public:
    /**
     * @brief Construct the class for a given @a axis axis type.
     */
    explicit S220Correlation(ShapeGeometry::Axis axis) : AxisCorrelation(axis) { }

    /**
     * @brief Returns "S110" as the signature name.
     */
    [[nodiscard]] std::string getSignatureName() const override { return "S110"; }
};


#endif //RAMPACK_S220CORRELATION_H
