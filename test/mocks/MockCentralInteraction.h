//
// Created by Piotr Kubala on 21/02/2024.
//

#ifndef RAMPACK_MOCKCENTRALINTERACTION_H
#define RAMPACK_MOCKCENTRALINTERACTION_H

#include <catch2/trompeloeil.hpp>

#include "core/interactions/CentralInteraction.h"


class MockCentralInteraction : public CentralInteraction {
protected:
    MAKE_CONST_MOCK1(calculateEnergyForDistance2, double(double), override);
};


#endif //RAMPACK_MOCKCENTRALINTERACTION_H
