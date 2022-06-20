//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_PARAMETERUPDATER_H
#define RAMPACK_PARAMETERUPDATER_H

#include <cstddef>

class ParameterUpdater {
public:
    virtual ~ParameterUpdater() = default;

    [[nodiscard]] virtual double getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const = 0;
};


#endif //RAMPACK_PARAMETERUPDATER_H
