//
// Created by Piotr Kubala on 29/12/2022.
//

#ifndef RAMPACK_MOCKSIMULATIONPLAYER_H
#define RAMPACK_MOCKSIMULATIONPLAYER_H

#include <catch2/trompeloeil.hpp>

#include "core/SimulationPlayer.h"


class MockSimulationPlayer : public trompeloeil::mock_interface<SimulationPlayer> {
public:
    IMPLEMENT_CONST_MOCK0(hasNext);
    IMPLEMENT_MOCK2(nextSnapshot);
    IMPLEMENT_MOCK0(reset);
    IMPLEMENT_MOCK2(lastSnapshot);
    IMPLEMENT_MOCK3(jumpToSnapshot);
    IMPLEMENT_CONST_MOCK0(getCurrentSnapshotCycles);
    IMPLEMENT_CONST_MOCK0(getTotalCycles);
    IMPLEMENT_CONST_MOCK0(getCycleStep);
    IMPLEMENT_CONST_MOCK0(getNumMolecules);
    IMPLEMENT_MOCK0(close);
};


#endif //RAMPACK_MOCKSIMULATIONPLAYER_H
