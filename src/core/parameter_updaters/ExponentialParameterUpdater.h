//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_EXPONENTIALPARAMETERUPDATER_H
#define RAMPACK_EXPONENTIALPARAMETERUPDATER_H

#include <cmath>

#include "core/ParameterUpdater.h"


class ExponentialParameterUpdater : public ParameterUpdater {
private:
    double initialValue{};
    double rate{};

public:
    ExponentialParameterUpdater(double initialValue, double rate) : initialValue{initialValue}, rate{rate} { }

    [[nodiscard]] double getValueForCycle(std::size_t currentCycle,
                                          [[maybe_unused]] std::size_t totalCycles) const override
    {
        return this->initialValue * std::exp(this->rate * static_cast<double>(currentCycle));
    }
};


#endif //RAMPACK_EXPONENTIALPARAMETERUPDATER_H
