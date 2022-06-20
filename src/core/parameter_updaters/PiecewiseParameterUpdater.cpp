//
// Created by pkua on 20.06.22.
//

#include <algorithm>

#include "PiecewiseParameterUpdater.h"
#include "utils/Assertions.h"
#include "ShiftedParameterUpdater.h"


PiecewiseParameterUpdater::PiecewiseParameterUpdater(PiecewiseParameterUpdater::UpdaterList updaters) {
    Expects(!updaters.empty());
    Expects(updaters.front().first == 0);
    Expects(std::is_sorted(updaters.begin(), updaters.end(), [](const auto &u1, const auto &u2) {
        return u1.first < u2.first;
    }));

    for (auto &updater : updaters) {
        auto shiftedUpdater = std::make_unique<ShiftedParameterUpdater>(updater.first, std::move(updater.second));
        this->updaters.emplace_back(updater.first, std::move(shiftedUpdater));
    }
}

double PiecewiseParameterUpdater::getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const {
    Expects(currentCycle <= totalCycles);
    Expects(totalCycles >= this->updaters.back().first);

    auto cycleComp = [](std::size_t cycle, const auto &updater) { return cycle < updater.first; };
    auto nextUpdaterIt = std::upper_bound(this->updaters.begin(), this->updaters.end(), currentCycle, cycleComp);
    std::size_t piecewiseTotalCycles = totalCycles;
    if (nextUpdaterIt != this->updaters.end())
        piecewiseTotalCycles = nextUpdaterIt->first;
    auto updaterIt = std::prev(nextUpdaterIt);
    const auto &updater = *(updaterIt->second);

    return updater.getValueForCycle(currentCycle, piecewiseTotalCycles);
}
