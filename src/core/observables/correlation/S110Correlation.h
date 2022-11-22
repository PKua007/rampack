//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_S110CORRELATION_H
#define RAMPACK_S110CORRELATION_H

#include "CorrelationFunction.h"


/**
 * @brief Calculates \f$ S_{110} \f$ correlation function for two shapes.
 * @details It is defined as \f$ \mathbf{\hat{a}}_i \cdot \mathbf{\hat{a}}_j \f$, where \f$ \mathbf{\hat{a}}_\alpha \f$
 * is either primary or secondary axis of \f$ \alpha^\mathrm{th} \f$ molecule.
 */
class S110Correlation : public CorrelationFunction {
public:
    /**
     * @brief Molecular axis to be used in the computation of S110.
     */
    enum class Axis {
        /** @brief Primary (long) molecular axis. */
        PRIMARY_AXIS,
        /** @brief Secondary molecular axis. */
        SECONDARY_AXIS,
        /** @brief Auxiliary (third) molecular axis. */
        AUXILIARY_AXIS
    };

private:
    Axis axis;

public:
    /**
     * @brief Construct the class for a given @a axis axis type.
     */
    explicit S110Correlation(Axis axis) : axis{axis} { }

    [[nodiscard]] double calculate(const Shape &shape1, const Shape &shape2,
                                   const ShapeTraits &shapeTraits) const override;

    /**
     * @brief Returns "S110" as the signature name.
     */
    [[nodiscard]] std::string getSignatureName() const override { return "S110"; }
};


#endif //RAMPACK_S110CORRELATION_H
