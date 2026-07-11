//
// Created by Codex on 12/07/2026.
//

#ifndef RAMPACK_MOCKEXTERNALFIELD_H
#define RAMPACK_MOCKEXTERNALFIELD_H

#include <catch2/trompeloeil.hpp>

#include "core/ExternalField.h"


class MockExternalField : public trompeloeil::mock_interface<ExternalField> {
public:
    IMPLEMENT_MOCK1(setupForShapeGeometry);
    IMPLEMENT_MOCK1(setupForBox);
    IMPLEMENT_CONST_MOCK2(calculateEnergy);
    IMPLEMENT_CONST_MOCK0(getContinuityAlongBoxAxes);
};


#endif //RAMPACK_MOCKEXTERNALFIELD_H
