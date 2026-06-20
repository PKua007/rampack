//
// Created by Codex on 20/06/2026.
//

#ifndef RAMPACK_MOCKMOVESAMPLER_H
#define RAMPACK_MOCKMOVESAMPLER_H

#include <catch2/trompeloeil.hpp>

#include "core/MoveSampler.h"


class MockMoveSampler : public trompeloeil::mock_interface<MoveSampler> {
public:
    IMPLEMENT_CONST_MOCK0(getName);
    IMPLEMENT_MOCK3(sampleMove);
    IMPLEMENT_CONST_MOCK1(getNumOfRequestedMoves);
    IMPLEMENT_MOCK0(increaseStepSize);
    IMPLEMENT_MOCK0(decreaseStepSize);
    IMPLEMENT_CONST_MOCK0(getStepSizes);
    IMPLEMENT_MOCK2(setStepSize);
};


#endif //RAMPACK_MOCKMOVESAMPLER_H
