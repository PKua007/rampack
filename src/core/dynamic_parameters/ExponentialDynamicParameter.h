//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_EXPONENTIALDYNAMICPARAMETER_H
#define RAMPACK_EXPONENTIALDYNAMICPARAMETER_H

#include <cmath>

#include "core/DynamicParameter.h"


class ExponentialDynamicParameter : public DynamicParameter {
private:
    double initialValue{};
    double rate{};

public:
    ExponentialDynamicParameter(double initialValue, double rate) : initialValue{initialValue}, rate{rate} { }

    [[nodiscard]] double getValueForCycle(std::size_t currentCycle,
                                          [[maybe_unused]] std::size_t totalCycles) const override
    {
        return this->initialValue * std::exp(this->rate * static_cast<double>(currentCycle));
    }
};


#endif //RAMPACK_EXPONENTIALDYNAMICPARAMETER_H
