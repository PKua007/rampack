//
// Created by Michal Ciesla on 10.06.2024.
//

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeTraits.h"
#include "matchers/VectorApproxMatcher.h"

#include "core/move_samplers/AxialRotationSampler.h"
#include "core/shapes/PolyspherocylinderBananaTraits.h"
#include "core/lattice/Lattice.h"
#include "core/PeriodicBoundaryConditions.h"


namespace {
    void test_axis_rotation_move(AxialRotationSampler &rotationSampler, const ShapeTraits &traits,
                                 const Vector<3> &invariantAxis)
    {
        Lattice lattice(UnitCell(TriclinicBox(2), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();
        Packing packing(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), traits.getInteraction(),
                        traits.getDataManager());
        rotationSampler.setup(packing, traits);
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
    ALLOW_CALL(sphereWithAxis, getRangeRadius(_)).RETURN(1);
    ALLOW_CALL(sphereWithAxis, getTotalRangeRadius(_)).RETURN(1);
    ALLOW_CALL(sphereWithAxis, overlapBetween(_, _, _, _, _, _, _, _, _)).RETURN(_9.getDistance2(_1, _5) < 1);
    ALLOW_CALL(sphereWithAxis, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(sphereWithAxis, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    ALLOW_CALL(sphereWithAxis, getShapeDataSize()).RETURN(0);

    SECTION("performing moves") {
        SECTION("global axis") {
            AxialRotationSampler rotationSampler(M_PI/2, Vector<3>{0, 0, 1});

            test_axis_rotation_move(rotationSampler, sphereWithAxis, {0, 0, 1});
        }

        SECTION("shape axis") {
            AxialRotationSampler rotationSampler(M_PI/2, ShapeGeometry::Axis::PRIMARY);

            test_axis_rotation_move(rotationSampler, sphereWithAxis, {1, 0, 0});
        }
    }

    SECTION("sampler names") {
        auto nameFor = [](const AxialRotationSampler::Axis &axis) {
            return AxialRotationSampler(M_PI/2, axis).getName();
        };

        SECTION("global axis") {
            CHECK(nameFor(Vector<3>{0, 0, 1}) == "axial_rotation(0,0,1)");
            CHECK(nameFor(Vector<3>{0.6, 0.8, 0}) == "axial_rotation(0.59999999999999998,0.80000000000000004,0)");
        }

        SECTION("shape axis") {
            CHECK(nameFor(ShapeGeometry::Axis::PRIMARY) == "axial_rotation(primary)");
            CHECK(nameFor(ShapeGeometry::Axis::SECONDARY) == "axial_rotation(secondary)");
            CHECK(nameFor(ShapeGeometry::Axis::AUXILIARY) == "axial_rotation(auxiliary)");
        }
    }

    SECTION("step sizes") {
        AxialRotationSampler rotationSampler(M_PI/2, Vector<3>{0, 0, 1});

        rotationSampler.setStepSize("rotation", 0.5);

        auto stepSizes = rotationSampler.getStepSizes();
        REQUIRE(stepSizes.size() == 1);
        CHECK(stepSizes[0].first == "rotation");
        CHECK(stepSizes[0].second == 0.5);
    }
}
