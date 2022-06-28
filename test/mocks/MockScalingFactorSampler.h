//
// Created by pkua on 28.06.22.
//

#ifndef RAMPACK_MOCKSCALINGFACTORSAMPLER_H
#define RAMPACK_MOCKSCALINGFACTORSAMPLER_H

#include <catch2/trompeloeil.hpp>

#include "core/volume_scalers/ScalingFactorSampler.h"


class MockScalingFactorSampler : public trompeloeil::mock_interface<ScalingFactorSampler> {
    IMPLEMENT_CONST_MOCK4(sampleFactors);
};


#endif //RAMPACK_MOCKSCALINGFACTORSAMPLER_H
