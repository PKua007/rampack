//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_SHIFTEDPARAMETERUPDATER_H
#define RAMPACK_SHIFTEDPARAMETERUPDATER_H

#include <memory>

#include "core/ParameterUpdater.h"


class ShiftedParameterUpdater : public ParameterUpdater {
private:
    bool isShiftNegative{};
    std::size_t cycleShift{};
    std::unique_ptr<ParameterUpdater> underlyingUpdater;

public:
    explicit ShiftedParameterUpdater(long cycleShift, std::unique_ptr<ParameterUpdater> underlyingUpdater);

    [[nodiscard]] double getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const override;
};


#endif //RAMPACK_SHIFTEDPARAMETERUPDATER_H
