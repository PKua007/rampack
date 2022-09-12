//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_POLARIZATIONCORRELATION_H
#define RAMPACK_POLARIZATIONCORRELATION_H

#include "CorrelationFunction.h"


class PolarizationCorrelation : public CorrelationFunction {
public:
    [[nodiscard]] double calculate(const Shape &shape1, const Shape &shape2) const override;
};


#endif //RAMPACK_POLARIZATIONCORRELATION_H
