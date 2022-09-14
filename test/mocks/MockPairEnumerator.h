//
// Created by pkua on 14.09.22.
//

#ifndef RAMPACK_MOCKPAIRENUMERATOR_H
#define RAMPACK_MOCKPAIRENUMERATOR_H

#include <catch2/trompeloeil.hpp>

#include "core/observables/correlation/PairEnumerator.h"


class MockPairEnumerator : public trompeloeil::mock_interface<PairEnumerator> {
    IMPLEMENT_CONST_MOCK3(enumeratePairs);
    IMPLEMENT_CONST_MOCK2(getExpectedNumOfMoleculesInShells);
    IMPLEMENT_CONST_MOCK0(getSignatureName);
};

#endif //RAMPACK_MOCKPAIRENUMERATOR_H
