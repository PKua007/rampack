//
// Created by pkua on 20.06.22.
//

#include <algorithm>

#include "PiecewiseDynamicParameter.h"
#include "utils/Exceptions.h"
#include "ShiftedDynamicParameter.h"


PiecewiseDynamicParameter::PiecewiseDynamicParameter(PiecewiseDynamicParameter::ParameterList updaters) {
    Expects(!updaters.empty());
    Expects(updaters.front().first == 0);
    Expects(std::is_sorted(updaters.begin(), updaters.end(), [](const auto &u1, const auto &u2) {
        return u1.first < u2.first;
    }));

    for (auto &updater : updaters) {
        auto shiftedUpdater = std::make_shared<ShiftedDynamicParameter>(updater.first, std::move(updater.second));
        this->updaters.emplace_back(updater.first, std::move(shiftedUpdater));
    }
}

double PiecewiseDynamicParameter::getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const {
    Expects(currentCycle <= totalCycles);
    Expects(totalCycles >= this->updaters.back().first);

    auto cycleComp = [](std::size_t cycle, const auto &updater) { return cycle < updater.first; };
    auto nextParamIt = std::upper_bound(this->updaters.begin(), this->updaters.end(), currentCycle, cycleComp);
    std::size_t piecewiseTotalCycles = totalCycles;
    if (nextParamIt != this->updaters.end())
        piecewiseTotalCycles = nextParamIt->first;
    auto paramIt = std::prev(nextParamIt);
    const auto &parameter = *(paramIt->second);

    return parameter.getValueForCycle(currentCycle, piecewiseTotalCycles);
}
