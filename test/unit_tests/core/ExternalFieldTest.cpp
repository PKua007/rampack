//
// Created by Codex on 12/07/2026.
//

#include <catch2/catch.hpp>

#include "mocks/MockExternalField.h"


TEST_CASE("ExternalField: particle selection") {
    SECTION("default selection is all particles") {
        MockExternalField field;

        CHECK(field.getParticleSelection().getMode() == ParticleSelection::Mode::ALL);
    }

    SECTION("particle selection can be set and prepared") {
        MockExternalField field;

        field.setParticleSelection(ParticleSelection::whitelist({2, 0}));
        field.getParticleSelection().prepare(4);

        CHECK(field.getParticleSelection().getMode() == ParticleSelection::Mode::WHITELIST);
        CHECK(field.getParticleSelection().getSpecifiedParticleIndices() == std::vector<std::size_t>{0, 2});
        CHECK(field.getParticleSelection().getActiveParticleIndices() == std::vector<std::size_t>{0, 2});
    }
}
