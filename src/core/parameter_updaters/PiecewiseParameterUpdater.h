//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_PIECEWISEPARAMETERUPDATER_H
#define RAMPACK_PIECEWISEPARAMETERUPDATER_H

#include <memory>
#include <vector>

#include "core/ParameterUpdater.h"


class PiecewiseParameterUpdater : public ParameterUpdater {
public:
    using UpdaterList = std::vector<std::pair<std::size_t, std::unique_ptr<ParameterUpdater>>>;

private:
     UpdaterList updaters;

public:
    explicit PiecewiseParameterUpdater(UpdaterList updaters);

    [[nodiscard]] double getValueForCycle(std::size_t currentCycle, std::size_t totalCycles) const override;
};


#endif //RAMPACK_PIECEWISEPARAMETERUPDATER_H
