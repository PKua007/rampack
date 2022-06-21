//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_PIECEWISEDYNAMICPARAMETER_H
#define RAMPACK_PIECEWISEDYNAMICPARAMETER_H

#include <memory>
#include <vector>

#include "core/DynamicParameter.h"


class PiecewiseDynamicParameter : public DynamicParameter {
public:
    using ParameterList = std::vector<std::pair<std::size_t, std::unique_ptr<DynamicParameter>>>;

private:
     ParameterList updaters;

public:
    explicit PiecewiseDynamicParameter(ParameterList updaters);

    [[nodiscard]] double getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const override;
};


#endif //RAMPACK_PIECEWISEDYNAMICPARAMETER_H
