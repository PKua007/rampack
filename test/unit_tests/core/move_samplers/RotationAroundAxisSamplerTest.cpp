//
// Created by Michal Ciesla on 10.06.2024.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/move_samplers/RotationWithAngleConservationSampler.h"
#include "core/move_samplers/RotationAroundAxisSampler.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/lattice/Lattice.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    void test_rotationAroundAxis_move(const ShapeTraits &traits) {
        Lattice lattice(UnitCell(TriclinicBox(2), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();

        Vector<3, double> globalAxis({0, 1, 1});
        Vector<3, double> particleAxis = traits.getGeometry().getPrimaryAxis({});
        Vector<3, double> zeroVector({0, 0, 0});

        globalAxis = globalAxis.normalized();
        Packing packing(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), traits.getInteraction());
        RotationAroundAxisSampler globalRotationSampler(1, globalAxis, true);
        RotationAroundAxisSampler particleRotationSampler(1, particleAxis, false);

        const auto &interaction = traits.getInteraction();

        globalRotationSampler.setupForShapeTraits(traits);
        particleRotationSampler.setupForShapeTraits(traits);

        std::vector<std::size_t> particleIdxs(packing.size());
        std::iota(particleIdxs.begin(), particleIdxs.end(), 0);
        std::mt19937 mt(1234);

        for (size_t i = 0; i < packing.size(); i++) {
            auto axis = packing[i].getOrientation() * traits.getGeometry().getPrimaryAxis({});
            auto product = axis * globalAxis;
            CHECK(std::fabs(product - globalAxis[2]) < 1e-12);
        }
        for (auto i = 0; i < 10000; i++) {
            auto move = globalRotationSampler.sampleMove(packing, particleIdxs, mt);
            CHECK(move.particleIdx < packing.size());
            CHECK(move.moveType == MoveSampler::MoveType::ROTATION);
            packing.tryRotation(move.particleIdx, move.rotation, interaction);
            packing.acceptRotation();

            move = particleRotationSampler.sampleMove(packing, particleIdxs, mt);
            CHECK(move.particleIdx < packing.size());
            CHECK(move.moveType == MoveSampler::MoveType::ROTATION);
            packing.tryRotation(move.particleIdx, move.rotation, interaction);
            packing.acceptRotation();

            CHECK_THAT(move.translation, IsApproxEqual(zeroVector, 1e-12));
        }

        for (size_t i = 0; i < packing.size(); i++) {
            auto axis = packing[i].getOrientation() * traits.getGeometry().getPrimaryAxis({});
            auto product = axis * globalAxis;
            CHECK(std::fabs(product - globalAxis[2]) < 1e-12);
        }
    }
}

TEST_CASE("RotationAroundAxisSampler") {
    PolyspherocylinderBananaTraits traits(5, 2 * M_PI / 3, 2, 0.1, 1);
    SECTION("move basic test") {
        test_rotationAroundAxis_move(traits);
    }
}
