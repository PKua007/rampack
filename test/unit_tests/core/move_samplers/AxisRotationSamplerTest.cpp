//
// Created by Michal Ciesla on 10.06.2024.
//

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeTraits.h"
#include "matchers/VectorApproxMatcher.h"

#include "core/move_samplers/AxisRotationSampler.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/lattice/Lattice.h"
#include "core/PeriodicBoundaryConditions.h"


namespace {
    void test_axis_rotation_move(AxisRotationSampler &rotationSampler, const ShapeTraits &traits,
                                 const Vector<3> &invariantAxis)
    {
        rotationSampler.setupForShapeTraits(traits);
        Lattice lattice(UnitCell(TriclinicBox(2), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();
        Packing packing(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), traits.getInteraction());
        std::vector<std::size_t> particleIdxs(packing.size());
        std::iota(particleIdxs.begin(), particleIdxs.end(), 0);
        std::mt19937 mt(1234); // NOLINT(*-msc51-cpp)

        auto move = rotationSampler.sampleMove(packing, particleIdxs, mt);

        CHECK(move.particleIdx < packing.size());
        CHECK(move.moveType == MoveSampler::MoveType::ROTATION);
        CHECK_THAT(move.rotation * invariantAxis, IsApproxEqual(invariantAxis, 1e-12));
        CHECK(move.rotation.tr() != Approx(3));
    }
}

TEST_CASE("AxisRotationSampler") {
    using trompeloeil::_;

    MockShapeTraits sphereWithAxis;
    ALLOW_CALL(sphereWithAxis, hasHardPart()).RETURN(true);
    ALLOW_CALL(sphereWithAxis, hasSoftPart()).RETURN(false);
    ALLOW_CALL(sphereWithAxis, getRangeRadius()).RETURN(1);
    ALLOW_CALL(sphereWithAxis, getTotalRangeRadius()).RETURN(1);
    ALLOW_CALL(sphereWithAxis, overlapBetween(_, _, _, _, _, _, _)).RETURN(_7.getDistance2(_1, _4) < 1);
    ALLOW_CALL(sphereWithAxis, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(sphereWithAxis, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});

    SECTION("global axis") {
        AxisRotationSampler rotationSampler(M_PI/2, Vector<3>{0, 0, 1});

        test_axis_rotation_move(rotationSampler, sphereWithAxis, {0, 0, 1});
    }

    SECTION("shape axis") {
        AxisRotationSampler rotationSampler(M_PI/2, ShapeGeometry::Axis::PRIMARY);

        test_axis_rotation_move(rotationSampler, sphereWithAxis, {1, 0, 0});
    }
}
