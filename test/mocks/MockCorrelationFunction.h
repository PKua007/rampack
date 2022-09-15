//
// Created by pkua on 15.09.22.
//

#ifndef RAMPACK_MOCKCORRELATIONFUNCTION_H
#define RAMPACK_MOCKCORRELATIONFUNCTION_H

#include <catch2/trompeloeil.hpp>

#include "core/observables/correlation/CorrelationFunction.h"


class MockCorrelationFunction : public trompeloeil::mock_interface<CorrelationFunction> {
public:
    IMPLEMENT_CONST_MOCK3(calculate);
    IMPLEMENT_CONST_MOCK0(getSignatureName);
};


#endif //RAMPACK_MOCKCORRELATIONFUNCTION_H
