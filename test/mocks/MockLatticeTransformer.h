//
// Created by Piotr Kubala on 22/05/2024.
//

#ifndef RAMPACK_MOCKLATTICETRANSFORMER_H
#define RAMPACK_MOCKLATTICETRANSFORMER_H

#include <catch2/trompeloeil.hpp>

#include "core/lattice/LatticeTransformer.h"


class MockLatticeTransformer : public trompeloeil::mock_interface<LatticeTransformer> {
public:
    IMPLEMENT_CONST_MOCK2(transform);
};


#endif //RAMPACK_MOCKLATTICETRANSFORMER_H
