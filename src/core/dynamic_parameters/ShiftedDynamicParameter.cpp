//
// Created by pkua on 20.06.22.
//

#include <limits>

#include "ShiftedDynamicParameter.h"
#include "utils/Assertions.h"


ShiftedDynamicParameter::ShiftedDynamicParameter(long cycleShift, std::shared_ptr<DynamicParameter> underlyingParameter)
        : underlyingParameter{std::move(underlyingParameter)}
{
    if (cycleShift > 0) {
        this->isShiftNegative = false;
        this->cycleShift = cycleShift;
    } else {
        this->isShiftNegative = true;
        this->cycleShift = -cycleShift;
    }

    Expects(this->underlyingParameter != nullptr);
}

double ShiftedDynamicParameter::getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const {
    if (this->isShiftNegative) {
        Expects(currentCycle <= std::numeric_limits<std::size_t>::max() - this->cycleShift);
        std::size_t newTotalCycles{};
        if (totalCycles > std::numeric_limits<std::size_t>::max() - this->cycleShift)
            newTotalCycles = std::numeric_limits<std::size_t>::max();
        else
            newTotalCycles = totalCycles + this->cycleShift;
        return this->underlyingParameter->getValueForCycle(currentCycle + this->cycleShift, newTotalCycles);
    } else {
        Expects(currentCycle >= this->cycleShift);
        Expects(totalCycles >= this->cycleShift);
        return this->underlyingParameter->getValueForCycle(currentCycle - this->cycleShift,
                                                           totalCycles - this->cycleShift);
    }
}
