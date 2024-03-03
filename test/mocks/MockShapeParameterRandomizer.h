//
// Created by Piotr Kubala on 03/03/2024.
//

#ifndef RAMPACK_MOCKSHAPEPARAMETERRANDOMIZER_H
#define RAMPACK_MOCKSHAPEPARAMETERRANDOMIZER_H

#include <catch2/trompeloeil.hpp>

#include "core/lattice/ShapeParameterRandomizer.h"


class MockShapeParameterRandomizer : public trompeloeil::mock_interface<ShapeParameterRandomizer> {
public:
    IMPLEMENT_CONST_MOCK2(randomize);
};


#endif //RAMPACK_MOCKSHAPEPARAMETERRANDOMIZER_H
