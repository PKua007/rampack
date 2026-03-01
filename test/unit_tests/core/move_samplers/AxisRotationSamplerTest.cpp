//
// Created by Michal Ciesla on 10.06.2024.
//

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeTraits.h"
#include "matchers/VectorApproxMatcher.h"

#include "core/move_samplers/AxialRotationSampler.h"
#include "core/lattice/Lattice.h"
#include "core/PeriodicBoundaryConditions.h"


namespace {
    void test_axis_rotation_move(AxialRotationSampler &rotationSampler, const ShapeTraits &traits, const Shape& shape,
                                 const Vector<3> &invariantAxis)
    {
        rotationSampler.setupForShapeTraits(traits);
        Lattice lattice(UnitCell(TriclinicBox(2), {shape}), {2, 2, 2});
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

    SECTION("performing moves") {
        // Shape rotated 90 deg around Z axis; consequently, shape axis primary=={1, 0, 0} is {0, 1, 0} in lab coords
        Shape shape({0.5, 0.5, 0.5}, Matrix<3, 3>::rotation(0, 0, M_PI/2));

        // Shape axis should react to shape orientation
        SECTION("shape axis") {
            SECTION("general") {
                AxialRotationSampler rotationSampler(M_PI/2, GeneralShapeAxis(Vector<3>{1, 0, 0}));

                test_axis_rotation_move(rotationSampler, sphereWithAxis, shape, {0, 1, 0});
            }

            SECTION("named") {
                AxialRotationSampler rotationSampler(M_PI/2, ShapeGeometry::Axis::PRIMARY);

                test_axis_rotation_move(rotationSampler, sphereWithAxis, shape, {0, 1, 0});
            }
        }

        // Global axis should not react to shape orientation
        SECTION("global axis") {
            AxialRotationSampler rotationSampler(M_PI/2, Vector<3>{1, 0, 0});

            test_axis_rotation_move(rotationSampler, sphereWithAxis, shape, {1, 0, 0});
        }
    }

    SECTION("sampler names") {
        auto nameFor = [](const auto &axis) {
            return AxialRotationSampler(M_PI/2, axis).getName();
        };

        SECTION("shape axis") {
            SECTION("general") {
                CHECK(nameFor(GeneralShapeAxis(Vector<3>{0, 0, 1})) == "axial_rotation(shape,0,0,1)");
                CHECK(nameFor(GeneralShapeAxis(Vector<3>{0.6, 0.8, 0}))
                      == "axial_rotation(shape,0.59999999999999998,0.80000000000000004,0)");
            }

            SECTION("named") {
                CHECK(nameFor(ShapeGeometry::Axis::PRIMARY) == "axial_rotation(primary)");
                CHECK(nameFor(ShapeGeometry::Axis::SECONDARY) == "axial_rotation(secondary)");
                CHECK(nameFor(ShapeGeometry::Axis::AUXILIARY) == "axial_rotation(auxiliary)");
            }
        }

        SECTION("global axis") {
            CHECK(nameFor(Vector<3>{0, 0, 1}) == "axial_rotation(0,0,1)");
            CHECK(nameFor(Vector<3>{3, 4, 0}) == "axial_rotation(0.59999999999999998,0.80000000000000004,0)");
        }
    }

    SECTION("step sizes") {
        AxialRotationSampler rotationSampler(M_PI/2, GeneralShapeAxis(Vector<3>{0, 0, 1}));

        rotationSampler.setStepSize("rotation", 0.5);

        auto stepSizes = rotationSampler.getStepSizes();
        REQUIRE(stepSizes.size() == 1);
        CHECK(stepSizes[0].first == "rotation");
        CHECK(stepSizes[0].second == 0.5);
    }
}
