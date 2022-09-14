//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_S110CORRELATION_H
#define RAMPACK_S110CORRELATION_H

#include "CorrelationFunction.h"


class S110Correlation : public CorrelationFunction {
public:
    enum class Axis {
        PRIMARY_AXIS,
        SECONDARY_AXIS
    };

private:
    Axis axis;

public:
    explicit S110Correlation(Axis axis) : axis{axis} { }

    [[nodiscard]] double calculate(const Shape &shape1, const Shape &shape2,
                                   const ShapeTraits &shapeTraits) const override;

    [[nodiscard]] std::string getSignatureName() const override { return "S110"; }
};


#endif //RAMPACK_S110CORRELATION_H
