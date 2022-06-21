//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_LINEARDYNAMICPARAMETER_H
#define RAMPACK_LINEARDYNAMICPARAMETER_H

#include "core/DynamicParameter.h"

/**
 * @brief Dynamic parameter changing linearly with the cycle number.
 * @details More precisely, the value is calculated as @a initialValue + @a slope * @a currentCycle). @a totalCycles is
 * ignored.
 */
class LinearDynamicParameter : public DynamicParameter {
private:
    double slope{};
    double initialValue{};

public:
    LinearDynamicParameter(double slope, double initialValue) : slope{slope}, initialValue{initialValue} { }

    [[nodiscard]] double getValueForCycle(std::size_t currentCycle,
                                          [[maybe_unused]] std::size_t totalCycles) const override
    {
        return this->initialValue + static_cast<double>(currentCycle) * this->slope;
    }
};


#endif //RAMPACK_LINEARDYNAMICPARAMETER_H
