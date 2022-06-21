//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_SHIFTEDDYNAMICPARAMETER_H
#define RAMPACK_SHIFTEDDYNAMICPARAMETER_H

#include <memory>

#include "core/DynamicParameter.h"


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
