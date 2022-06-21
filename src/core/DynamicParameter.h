//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_DYNAMICPARAMETER_H
#define RAMPACK_DYNAMICPARAMETER_H

#include <cstddef>


/**
 * @brief An interface representing some kind of @a double parameter, whose value changes with the cycle number.
 */
class DynamicParameter {
public:
    virtual ~DynamicParameter() = default;

    /**
     * @brief Returns parameter's value for @a currentCycle (possibly using also total number of cycles @a totalCycles).
     * @details @a currentCycle does not necessarily have to be smaller than @a totalCycles, but specific parameters
     * are allowed to throw and exception in that case.
     */
    [[nodiscard]] virtual double getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const = 0;
};


#endif //RAMPACK_DYNAMICPARAMETER_H
