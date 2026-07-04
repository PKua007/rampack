//
// Created by Codex on 14/06/2026.
//

#include <catch2/catch.hpp>

#include "core/ParticleSelection.h"
#include "utils/Exceptions.h"


TEST_CASE("ParticleSelection: creation") {
    SECTION("default selection activates all particles without storing indices") {
        ParticleSelection selection;

        selection.prepare(4);

        CHECK(selection.getMode() == ParticleSelection::Mode::ALL);
        CHECK(selection.getSpecifiedParticleIndices().empty());
        CHECK(selection.getActiveParticleIndices() == std::vector<std::size_t>{0, 1, 2, 3});
        CHECK(selection.getNumActiveParticles() == 4);
        CHECK(selection.isParticleActive(0));
        CHECK(selection.isParticleActive(1));
        CHECK(selection.isParticleActive(2));
        CHECK(selection.isParticleActive(3));
    }

    SECTION("whitelist activates only listed particles") {
        auto selection = ParticleSelection::whitelist({3, 1});

        selection.prepare(5);

        CHECK(selection.getMode() == ParticleSelection::Mode::WHITELIST);
        CHECK(selection.getSpecifiedParticleIndices() == std::vector<std::size_t>{1, 3});
        CHECK(selection.getActiveParticleIndices() == std::vector<std::size_t>{1, 3});
        CHECK(selection.getNumActiveParticles() == 2);
        CHECK_FALSE(selection.isParticleActive(0));
        CHECK(selection.isParticleActive(1));
        CHECK_FALSE(selection.isParticleActive(2));
        CHECK(selection.isParticleActive(3));
        CHECK_FALSE(selection.isParticleActive(4));
    }

    SECTION("blacklist activates all particles except listed particles") {
        auto selection = ParticleSelection::blacklist({0, 2});

        selection.prepare(4);

        CHECK(selection.getMode() == ParticleSelection::Mode::BLACKLIST);
        CHECK(selection.getSpecifiedParticleIndices() == std::vector<std::size_t>{0, 2});
        CHECK(selection.getActiveParticleIndices() == std::vector<std::size_t>{1, 3});
        CHECK(selection.getNumActiveParticles() == 2);
        CHECK_FALSE(selection.isParticleActive(0));
        CHECK(selection.isParticleActive(1));
        CHECK_FALSE(selection.isParticleActive(2));
        CHECK(selection.isParticleActive(3));
    }
}

TEST_CASE("ParticleSelection: edge cases") {
    SECTION("unprepared selection has no active particles and cannot answer particle activity") {
        ParticleSelection selection;

        CHECK(selection.getSpecifiedParticleIndices().empty());
        CHECK(selection.getActiveParticleIndices().empty());
        CHECK(selection.getNumActiveParticles() == 0);
        CHECK_THROWS_AS(selection.isParticleActive(0), PreconditionException);
    }

    SECTION("empty whitelist is valid and has no active particles") {
        auto selection = ParticleSelection::whitelist({});

        selection.prepare(3);

        CHECK(selection.getSpecifiedParticleIndices().empty());
        CHECK(selection.getActiveParticleIndices().empty());
        CHECK(selection.getNumActiveParticles() == 0);
        CHECK_FALSE(selection.isParticleActive(0));
        CHECK_FALSE(selection.isParticleActive(1));
        CHECK_FALSE(selection.isParticleActive(2));
    }

    SECTION("empty blacklist is valid and activates all particles") {
        auto selection = ParticleSelection::blacklist({});

        selection.prepare(3);

        CHECK(selection.getSpecifiedParticleIndices().empty());
        CHECK(selection.getActiveParticleIndices() == std::vector<std::size_t>{0, 1, 2});
        CHECK(selection.getNumActiveParticles() == 3);
        CHECK(selection.isParticleActive(0));
        CHECK(selection.isParticleActive(1));
        CHECK(selection.isParticleActive(2));
    }

    SECTION("prepare skips indices outside the particle range") {
        auto selection = ParticleSelection::whitelist({0, 3});

        selection.prepare(3);

        CHECK(selection.getSpecifiedParticleIndices() == std::vector<std::size_t>{0, 3});
        CHECK(selection.getActiveParticleIndices() == std::vector<std::size_t>{0});
        CHECK(selection.getNumActiveParticles() == 1);
        CHECK(selection.isParticleActive(0));
        CHECK_FALSE(selection.isParticleActive(1));
        CHECK_FALSE(selection.isParticleActive(2));
    }

    SECTION("constructor sorts and removes duplicate specified indices") {
        auto selection = ParticleSelection::blacklist({4, 1, 4, 2, 1});

        selection.prepare(6);

        CHECK(selection.getSpecifiedParticleIndices() == std::vector<std::size_t>{1, 2, 4});
        CHECK(selection.getActiveParticleIndices() == std::vector<std::size_t>{0, 3, 5});
    }
}

TEST_CASE("ParticleSelection: errors") {
    SECTION("particle activity queries validate particle index") {
        ParticleSelection selection;
        selection.prepare(2);

        CHECK_THROWS_AS(selection.isParticleActive(2), PreconditionException);
    }
}
