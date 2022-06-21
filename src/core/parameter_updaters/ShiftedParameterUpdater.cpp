//
// Created by pkua on 20.06.22.
//

#include <limits>

#include "ShiftedParameterUpdater.h"
#include "utils/Assertions.h"


ShiftedParameterUpdater::ShiftedParameterUpdater(long cycleShift, std::unique_ptr<ParameterUpdater> underlyingUpdater)
        : underlyingUpdater{std::move(underlyingUpdater)}
{
    if (cycleShift > 0) {
        this->isShiftNegative = false;
        this->cycleShift = cycleShift;
    } else {
        this->isShiftNegative = true;
        this->cycleShift = -cycleShift;
    }

    Expects(this->underlyingUpdater != nullptr);
}

double ShiftedParameterUpdater::getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const {
    if (this->isShiftNegative) {
        Expects(currentCycle <= std::numeric_limits<std::size_t>::max() - this->cycleShift);
        std::size_t newTotalCycles{};
        if (totalCycles > std::numeric_limits<std::size_t>::max() - this->cycleShift)
            newTotalCycles = std::numeric_limits<std::size_t>::max();
        else
            newTotalCycles = totalCycles + this->cycleShift;
        return this->underlyingUpdater->getValueForCycle(currentCycle + this->cycleShift, newTotalCycles);
    } else {
        Expects(currentCycle >= this->cycleShift);
        Expects(totalCycles >= this->cycleShift);
        return this->underlyingUpdater->getValueForCycle(currentCycle - this->cycleShift,
                                                         totalCycles - this->cycleShift);
    }
}
