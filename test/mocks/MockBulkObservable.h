//
// Created by pkua on 20.09.22.
//

#ifndef RAMPACK_MOCKBULKOBSERVABLE_H
#define RAMPACK_MOCKBULKOBSERVABLE_H

#include <catch2/trompeloeil.hpp>

#include "core/BulkObservable.h"


class MockBulkObservable : public trompeloeil::mock_interface<BulkObservable> {
public:
    IMPLEMENT_MOCK4(addSnapshot);
    IMPLEMENT_CONST_MOCK1(print);
    IMPLEMENT_MOCK0(clear);
    IMPLEMENT_CONST_MOCK0(getSignatureName);
};


#endif //RAMPACK_MOCKBULKOBSERVABLE_H
