//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_CONSTANTDYNAMICPARAMETER_H
#define RAMPACK_CONSTANTDYNAMICPARAMETER_H

#include "core/DynamicParameter.h"


/**
 * @brief Constant parameter, which always returns the same value passed in the constructor, regardless of the cycle
 * number.
 */
class ConstantDynamicParameter : public DynamicParameter {
private:
    double value{};

public:
    explicit ConstantDynamicParameter(double value) : value{value} { }

    [[nodiscard]] double getValueForCycle([[maybe_unused]] std::size_t currentCycle,
                                          [[maybe_unused]] std::size_t totalCycles) const override
    {
        return this->value;
    }
};


#endif //RAMPACK_CONSTANTDYNAMICPARAMETER_H
