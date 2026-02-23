//
// Created by Michal Ciesla on 14.01.25.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/move_samplers/ReflectionSampler.h"
#include "core/move_samplers/RotationSampler.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/lattice/Lattice.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    void test_rotationAroundAxis_move([[maybe_unused]]  const ShapeTraits &traits) {
        /*Lattice lattice(UnitCell(TriclinicBox(2), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();

        Vector<3, double> zeroVector({0, 0, 0});
        Packing packing(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), traits.getInteraction());
        RotationSampler rotation(M_PI_2);
        ReflectionSampler reflectionX(1, Vector<3, double>({1, 0, 0}));
        reflectionX.setupForShapeTraits(traits);
        ReflectionSampler reflectionY(1, Vector<3, double>({0, 1, 0}));
        reflectionY.setupForShapeTraits(traits);
        ReflectionSampler reflectionZ(1, Vector<3, double>({0, 0, 1}));
        reflectionZ.setupForShapeTraits(traits);

        const auto &interaction = traits.getInteraction();

        std::vector<std::size_t> particleIdxs(packing.size());
        std::iota(particleIdxs.begin(), particleIdxs.end(), 0);
        std::mt19937 mt(1234);

        for (auto i=0; i<100; i++){
            auto move = rotation.sampleMove(packing, particleIdxs, mt);
            packing.tryRotation(move.particleIdx, move.rotation, interaction);
            packing.acceptRotation();
        }

        for (auto i = 0; i < 100; i++) {
            auto moveX = reflectionX.sampleMove(packing, particleIdxs, mt);

            CHECK(moveX.particleIdx < packing.size());
            CHECK(moveX.moveType == MoveSampler::MoveType::ROTATION);
            auto axis =  traits.getGeometry().getPrimaryAxis(packing[moveX.particleIdx]);
            packing.tryRotation(moveX.particleIdx, moveX.rotation, interaction);
            packing.acceptRotation();
            auto axisX = traits.getGeometry().getPrimaryAxis(packing[moveX.particleIdx]);
            REQUIRE_THAT(axis[0]+axisX[0], Catch::Matchers::WithinAbs(0, 1e-12));
            REQUIRE_THAT(axis[1]-axisX[1], Catch::Matchers::WithinAbs(0, 1e-12));
            REQUIRE_THAT(axis[2]-axisX[2], Catch::Matchers::WithinAbs(0, 1e-12));

            auto moveY = reflectionY.sampleMove(packing, particleIdxs, mt);
            axis =  traits.getGeometry().getPrimaryAxis(packing[moveY.particleIdx]);
            packing.tryRotation(moveY.particleIdx, moveY.rotation, interaction);
            packing.acceptRotation();
            auto axisY = traits.getGeometry().getPrimaryAxis(packing[moveY.particleIdx]);
            REQUIRE_THAT(axis[0]-axisY[0], Catch::Matchers::WithinAbs(0, 1e-12));
            REQUIRE_THAT(axis[1]+axisY[1], Catch::Matchers::WithinAbs(0, 1e-12));
            REQUIRE_THAT(axis[2]-axisY[2], Catch::Matchers::WithinAbs(0, 1e-12));

            auto moveZ = reflectionZ.sampleMove(packing, particleIdxs, mt);
            axis = traits.getGeometry().getPrimaryAxis(packing[moveZ.particleIdx]);
            packing.tryRotation(moveZ.particleIdx, moveZ.rotation, interaction);
            packing.acceptRotation();
            auto axisZ = traits.getGeometry().getPrimaryAxis(packing[moveZ.particleIdx]);
            REQUIRE_THAT(axis[0]-axisZ[0], Catch::Matchers::WithinAbs(0, 1e-12));
            REQUIRE_THAT(axis[1]-axisZ[1], Catch::Matchers::WithinAbs(0, 1e-12));
            REQUIRE_THAT(axis[2]+axisZ[2], Catch::Matchers::WithinAbs(0, 1e-12));

            CHECK_THAT(moveX.translation, IsApproxEqual(zeroVector, 1e-12));
            CHECK_THAT(moveY.translation, IsApproxEqual(zeroVector, 1e-12));
            CHECK_THAT(moveZ.translation, IsApproxEqual(zeroVector, 1e-12));
        }*/
    }
}

TEST_CASE("ReflectionSampler") {
    PolyspherocylinderBananaTraits traits(5, 2 * M_PI / 3, 2, 0.1, 1);
    SECTION("move basic test") {
        test_rotationAroundAxis_move(traits);
    }
}
