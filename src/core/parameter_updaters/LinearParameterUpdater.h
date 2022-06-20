//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_LINEARPARAMETERUPDATER_H
#define RAMPACK_LINEARPARAMETERUPDATER_H

#include "core/ParameterUpdater.h"


class LinearParameterUpdater : public ParameterUpdater {
private:
    double slope{};
    double initialValue{};

public:
    LinearParameterUpdater(double slope, double initialValue) : slope{slope}, initialValue{initialValue} { }

    [[nodiscard]] double getValueForCycle(std::size_t currentCycle,
                                          [[maybe_unused]] std::size_t totalCycles) const override
    {
        return this->initialValue + static_cast<double>(currentCycle) * this->slope;
    }
};


#endif //RAMPACK_LINEARPARAMETERUPDATER_H
