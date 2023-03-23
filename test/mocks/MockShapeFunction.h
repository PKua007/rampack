//
// Created by Piotr Kubala on 23/03/2023.
//

#ifndef RAMPACK_MOCKSHAPEFUNCTION_H
#define RAMPACK_MOCKSHAPEFUNCTION_H

#include <catch2/trompeloeil.hpp>

#include "core/observables/ShapeFunction.h"


class MockShapeFunction : public trompeloeil::mock_interface<ShapeFunction> {
public:
    IMPLEMENT_CONST_MOCK2(calculate);
    IMPLEMENT_CONST_MOCK0(getName);
};


#endif //RAMPACK_MOCKSHAPEFUNCTION_H
