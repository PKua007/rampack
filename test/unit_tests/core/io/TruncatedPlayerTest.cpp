//
// Created by Piotr Kubala on 29/12/2022.
//

#include <catch2/catch.hpp>

#include "mocks/MockSimulationPlayer.h"
#include "mocks/MockShapeTraits.h"

#include "core/io/TruncatedPlayer.h"
#include "core/Packing.h"
#include "core/PeriodicBoundaryConditions.h"


TEST_CASE("TruncatedPlayer") {
    using trompeloeil::_;

    auto mockPlayer = std::make_unique<MockSimulationPlayer>();
    MockSimulationPlayer &mockPlayerRef = *mockPlayer;
    std::size_t currentSnapshot{};
    ALLOW_CALL(mockPlayerRef, getTotalCycles()).RETURN(4000);
    ALLOW_CALL(mockPlayerRef, getCycleStep()).RETURN(1000);
    ALLOW_CALL(mockPlayerRef, getNumMolecules()).RETURN(500);
    ALLOW_CALL(mockPlayerRef, hasNext()).RETURN(currentSnapshot < 4000);
    ALLOW_CALL(mockPlayerRef, getCurrentSnapshotCycles()).LR_RETURN(currentSnapshot);

    std::unique_ptr<TruncatedPlayer> truncatedPlayer;

    // Resetting the underlying player on construction
    {
        REQUIRE_CALL(mockPlayerRef, reset()).LR_SIDE_EFFECT(currentSnapshot = 0);
        truncatedPlayer = std::make_unique<TruncatedPlayer>(std::move(mockPlayer), 2000);
    }

    // Basic info
    REQUIRE(truncatedPlayer->getTotalCycles() == 2000);
    REQUIRE(truncatedPlayer->getCycleStep() == 1000);
    REQUIRE(truncatedPlayer->getNumMolecules() == 500);

    Packing packing(std::make_unique<PeriodicBoundaryConditions>());
    MockShapeTraits traits;

    // Traversing the recording
    {
        REQUIRE_CALL(mockPlayerRef, nextSnapshot(_, _)).TIMES(2).LR_SIDE_EFFECT(currentSnapshot += 1000);
        REQUIRE(truncatedPlayer->hasNext());
        REQUIRE_NOTHROW(truncatedPlayer->nextSnapshot(packing, traits));
        REQUIRE(truncatedPlayer->hasNext());
        REQUIRE_NOTHROW(truncatedPlayer->nextSnapshot(packing, traits));
        REQUIRE_FALSE(truncatedPlayer->hasNext());
        REQUIRE_THROWS(truncatedPlayer->nextSnapshot(packing, traits));
    }

    // Jump to snapshot
    {
        REQUIRE_CALL(mockPlayerRef, jumpToSnapshot(_, _, 1000ul)).LR_SIDE_EFFECT(currentSnapshot = 1000);
        truncatedPlayer->jumpToSnapshot(packing, traits, 1000);
        REQUIRE(truncatedPlayer->getCurrentSnapshotCycles() == 1000);
    }

    // Reset
    {
        REQUIRE_CALL(mockPlayerRef, reset()).LR_SIDE_EFFECT(currentSnapshot = 0);
        truncatedPlayer->reset();
        REQUIRE(truncatedPlayer->getCurrentSnapshotCycles() == 0);
    }
}