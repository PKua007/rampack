//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_MOCKPARAMETERUPDATER_H
#define RAMPACK_MOCKPARAMETERUPDATER_H

#include <catch2/trompeloeil.hpp>

#include "core/ParameterUpdater.h"


class MockParameterUpdater : public trompeloeil::mock_interface<ParameterUpdater> {
public:
    IMPLEMENT_CONST_MOCK2(getValueForCycle);
};

#endif //RAMPACK_MOCKPARAMETERUPDATER_H