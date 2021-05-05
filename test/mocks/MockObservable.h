//
// Created by Piotr Kubala on 05/05/2021.
//

#ifndef RAMPACK_MOCKOBSERVABLE_H
#define RAMPACK_MOCKOBSERVABLE_H

#include <catch2/trompeloeil.hpp>

#include "core/Observable.h"

class MockObservable : public trompeloeil::mock_interface<Observable> {
    IMPLEMENT_MOCK4(calculate);
    IMPLEMENT_CONST_MOCK0(getIntervalHeader);
    IMPLEMENT_CONST_MOCK0(getNominalHeader);
    IMPLEMENT_CONST_MOCK0(getIntervalValues);
    IMPLEMENT_CONST_MOCK0(getNominalValues);
    IMPLEMENT_CONST_MOCK0(getName);
};

#endif //RAMPACK_MOCKOBSERVABLE_H
