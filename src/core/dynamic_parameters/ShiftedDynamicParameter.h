//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_SHIFTEDDYNAMICPARAMETER_H
#define RAMPACK_SHIFTEDDYNAMICPARAMETER_H

#include <memory>

#include "core/DynamicParameter.h"


/**
 * @brief A decorator for some underlying DynamicParameter, which shifts the number of cycles.
 * @details @a cycleShift from the constructor tells which cycle should be considered by underlying parameter as 0.
 * Thus, it is subtracted from both @a currentCycle and @a totalCycles. If the shift is positive and subtracting it
 * from any of: @a currentCycle, @a totalCycles would yield a negative number, and error is reported. For negative
 * shift value, error is reported if @a currentCycle would overflow @a std::size_t limit. The overflow of @a totalCycles
 * is not reported as an error, but in such a case @a totalCycles is capped at the maximal @a std::size_t value (which
 * is sometimes used to signify an unlimited number of cycles).
 */
class ShiftedDynamicParameter : public DynamicParameter {
private:
    bool isShiftNegative{};
    std::size_t cycleShift{};
    std::unique_ptr<DynamicParameter> underlyingParameter;

public:
    explicit ShiftedDynamicParameter(long cycleShift, std::unique_ptr<DynamicParameter> underlyingParameter);

    [[nodiscard]] double getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const override;
};


#endif //RAMPACK_SHIFTEDDYNAMICPARAMETER_H
