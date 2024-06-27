//
// Created by Piotr Kubala on 22/05/2024.
//

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockLatticeTransformer.h"

#include "matchers/PackingApproxPositionsCatchMatcher.h"

#include "core/io/RamtrjPlayer.h"
#include "core/io/RamtrjRecorder.h"
#include "core/io/TransformingPlayer.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("TransformingPlayer") {
    using trompeloeil::_;

    // Prepare packing
    SphereTraits traits(0.5);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> originalShapes{Shape({0.5, 0.5, 0.5})};
    Packing packing(TriclinicBox(1), std::move(originalShapes), std::move(bc), traits.getInteraction(),
                    traits.getDataManager());

    // Prepare transformer adding shape at {0.3, 0.3, 0.3}
    auto mockTransformer = std::make_shared<MockLatticeTransformer>();
    ALLOW_CALL(*mockTransformer, transform(_, _)).SIDE_EFFECT(
        const SphereTraits::HardData RADIUS = {0.5};
        const auto noRot = Matrix<3, 3>::identity();
        _1.modifyUnitCellMolecules().emplace_back(Vector<3>{0.3, 0.3, 0.3}, noRot, RADIUS);
    );
    std::vector<std::shared_ptr<LatticeTransformer>> transformers{mockTransformer};

    SECTION("empty") {
        // Prepare recorder
        std::stringbuf inout_buf;
        auto inout = std::make_unique<std::iostream>(&inout_buf);
        RamtrjRecorder recorder(std::move(inout), packing, traits.getDataManager(), 1000, false);
        recorder.close();

        // Prepare packing
        std::vector<Shape> shapes(2, Shape({0.1, 0.1, 0.1}));
        packing.reset(std::move(shapes), TriclinicBox(1), traits.getInteraction(), traits.getDataManager());

        // Prepare player
        inout = std::make_unique<std::iostream>(&inout_buf);
        auto originalPlayer = std::make_unique<RamtrjPlayer>(std::move(inout), traits.getDataManager());
        TransformingPlayer transformingPlayer(std::move(originalPlayer), std::move(transformers), packing, traits);

        CHECK(transformingPlayer.getTotalCycles() == 0);
        CHECK(transformingPlayer.getCycleStep() == 1000);
        CHECK(transformingPlayer.getNumMolecules() == 3);
        CHECK_FALSE(transformingPlayer.hasNext());
    }

    SECTION("non-empty") {
        // Prepare recording - packing with a single shape, first at {0.1, 0.1, 0.1}, then at {0.6, 0.6, 0.6}
        std::stringbuf inout_buf;
        auto inout = std::make_unique<std::iostream>(&inout_buf);
        RamtrjRecorder recorder(std::move(inout), packing, traits.getDataManager(), 1000, false);
        packing.reset({Shape({0.1, 0.1, 0.1})}, TriclinicBox(1), traits.getInteraction(), traits.getDataManager());
        recorder.recordSnapshot(packing, traits, 1000);
        packing.reset({Shape({0.6, 0.6, 0.6})}, TriclinicBox(1), traits.getInteraction(), traits.getDataManager());
        recorder.recordSnapshot(packing, traits, 2000);
        recorder.close();

        // Imbue a different number of shapes into the packing
        std::vector<Shape> shapes(3, Shape({0.1, 0.1, 0.1}));
        packing.reset(std::move(shapes), TriclinicBox(1), traits.getInteraction(), traits.getDataManager());

        // Prepare player
        inout = std::make_unique<std::iostream>(&inout_buf);
        auto originalPlayer = std::make_unique<RamtrjPlayer>(std::move(inout), traits.getDataManager());
        originalPlayer->lastSnapshot(packing, traits);    // Jump to last snapshot
        TransformingPlayer transformingPlayer(std::move(originalPlayer), std::move(transformers), packing, traits);

        SECTION("reset on construction") {
            CHECK(transformingPlayer.getCurrentSnapshotCycles() == 0);
        }

        SECTION("basic info") {
            CHECK(transformingPlayer.getTotalCycles() == 2000);
            CHECK(transformingPlayer.getCycleStep() == 1000);
            CHECK(transformingPlayer.getNumMolecules() == 2);
        }

        SECTION("traversing the recording") {
            REQUIRE(transformingPlayer.hasNext());
            REQUIRE_NOTHROW(transformingPlayer.nextSnapshot(packing, traits));
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.3, 0.3, 0.3}}, 1e-12));
            CHECK(transformingPlayer.getCurrentSnapshotCycles() == 1000);

            REQUIRE(transformingPlayer.hasNext());
            REQUIRE_NOTHROW(transformingPlayer.nextSnapshot(packing, traits));
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.6, 0.6, 0.6},
                                                                 {0.3, 0.3, 0.3}}, 1e-12));
            CHECK(transformingPlayer.getCurrentSnapshotCycles() == 2000);

            REQUIRE_FALSE(transformingPlayer.hasNext());
            REQUIRE_THROWS(transformingPlayer.nextSnapshot(packing, traits));
        }

        SECTION("jump to snapshot") {
            transformingPlayer.jumpToSnapshot(packing, traits, 1000);
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.3, 0.3, 0.3}}, 1e-12));
            REQUIRE(transformingPlayer.getCurrentSnapshotCycles() == 1000);
        }

        SECTION("last snapshot") {
            transformingPlayer.lastSnapshot(packing, traits);
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.6, 0.6, 0.6}, {0.3, 0.3, 0.3}}, 1e-12));
            REQUIRE(transformingPlayer.getCurrentSnapshotCycles() == 2000);
        }

        SECTION("reset") {
            transformingPlayer.reset();
            REQUIRE(transformingPlayer.getCurrentSnapshotCycles() == 0);
        }
    }
}