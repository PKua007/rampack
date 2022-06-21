//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_MOCKDYNAMICPARAMETER_H
#define RAMPACK_MOCKDYNAMICPARAMETER_H

#include <catch2/trompeloeil.hpp>

#include "core/DynamicParameter.h"


class MockDynamicParameter : public trompeloeil::mock_interface<DynamicParameter> {
public:
    IMPLEMENT_CONST_MOCK2(getValueForCycle);
};

#endif //RAMPACK_MOCKDYNAMICPARAMETER_H