//
// Created by pkua on 20.06.22.
//

#include "ShiftedParameterUpdater.h"
#include "utils/Assertions.h"


ShiftedParameterUpdater::ShiftedParameterUpdater(std::size_t cycleShift,
                                                 std::unique_ptr<ParameterUpdater> underlyingUpdater)
        : cycleShift{cycleShift}, underlyingUpdater{std::move(underlyingUpdater)}
{
    Expects(this->underlyingUpdater != nullptr);
}

double ShiftedParameterUpdater::getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const {
    return this->underlyingUpdater->getValueForCycle(currentCycle - this->cycleShift, totalCycles - this->cycleShift);
}
