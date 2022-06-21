//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_DYNAMICPARAMETER_H
#define RAMPACK_DYNAMICPARAMETER_H

#include <cstddef>


class DynamicParameter {
public:
    virtual ~DynamicParameter() = default;

    [[nodiscard]] virtual double getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const = 0;
};


#endif //RAMPACK_DYNAMICPARAMETER_H
