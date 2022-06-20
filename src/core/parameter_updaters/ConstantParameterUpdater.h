//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_CONSTANTPARAMETERUPDATER_H
#define RAMPACK_CONSTANTPARAMETERUPDATER_H

#include "core/ParameterUpdater.h"


class ConstantParameterUpdater : public ParameterUpdater {
private:
    double value{};

public:
    explicit ConstantParameterUpdater(double value) : value{value} { }

    [[nodiscard]] double getValueForCycle([[maybe_unused]] std::size_t currentCycle,
                                          [[maybe_unused]] std::size_t totalCycles) const override
    {
        return this->value;
    }
};


#endif //RAMPACK_CONSTANTPARAMETERUPDATER_H
