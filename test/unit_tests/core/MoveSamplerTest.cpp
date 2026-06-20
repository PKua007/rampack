//
// Created by Codex on 20/06/2026.
//

#include <catch2/catch.hpp>

#include "mocks/MockMoveSampler.h"


TEST_CASE("MoveSampler: particle selection") {
    SECTION("default selection activates all particles after preparation") {
        MockMoveSampler sampler;

        CHECK(sampler.getParticleSelection().getMode() == ParticleSelection::Mode::ALL);
    }
}
