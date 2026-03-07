//
// Created by Michal Ciesla on 14.01.25.
//

#include <algorithm>
#include <cmath>
#include <numeric>

#include <catch2/catch.hpp>

#include "matchers/InteractionCentresApproxMatcher.h"

#include "mocks/MockShapeTraits.h"

#include "core/PeriodicBoundaryConditions.h"
#include "core/lattice/Lattice.h"
#include "core/move_samplers/ReflectionSampler.h"

namespace {
    Packing construct_packing(const ShapeTraits &traits, const Shape &shape, const std::array<std::size_t, 3>& latticeDimensions)
    {
        Lattice lattice(UnitCell(TriclinicBox(5), {shape}), latticeDimensions);
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();
        return Packing(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), traits.getInteraction());
    }

    Shape apply_reflection_and_return_shape(Packing &packing, ReflectionSampler &reflectionSampler,
                                      const Interaction &interaction)
    {
        std::vector<std::size_t> particleIdxs(packing.size());
        std::iota(particleIdxs.begin(), particleIdxs.end(), 0);
        std::mt19937 mt(1234); // NOLINT(*-msc51-cpp)

        const auto move = reflectionSampler.sampleMove(packing, particleIdxs, mt);

        REQUIRE(move.particleIdx < packing.size());
        REQUIRE(move.moveType == MoveSampler::MoveType::ROTOTRANSLATION);

        packing.tryMove(move.particleIdx, move.translation, move.rotation, interaction);
        packing.acceptMove();

        return packing[move.particleIdx];
    }

    double orientation_orthogonality_error2(const Matrix<3, 3> &orientation)
    {
        const Matrix<3, 3> orthogonalityError
            = Matrix<3, 3>::identity() - orientation.transpose() * orientation;
        return orthogonalityError.norm2();
    }

    // Builds a small packing, applies one sampled reflection move, and verifies that the reflection was correctly
    // applied by inspecting interaction centers after reflection.
    void test_reflection_move(const ShapeTraits &traits, ReflectionSampler &reflectionSampler, const Shape &shape,
                              const std::vector<Vector<3>> &expectedRelativeCentresAfterReflection_)
    {
        const auto &interaction = traits.getInteraction();
        const auto &geometry = traits.getGeometry();

        auto packing = construct_packing(traits, shape, {2, 2, 2});

        const Shape reflectedShape = apply_reflection_and_return_shape(packing, reflectionSampler, interaction);
        const auto reflectedShapeInteractionCentres = interaction.getInteractionCentresForShape(reflectedShape);
        const auto reflectedShapeGeometricOrigin
            = reflectedShape.getPosition() + geometry.getGeometricOrigin(reflectedShape);

        std::vector<Vector<3>> relativeCentres(reflectedShapeInteractionCentres.size());
        const auto toRelativeCentre = [&reflectedShapeGeometricOrigin](const auto &centre) {
            return centre - reflectedShapeGeometricOrigin;
        };
        std::transform(reflectedShapeInteractionCentres.begin(), reflectedShapeInteractionCentres.end(),
                       relativeCentres.begin(), toRelativeCentre);

        CHECK_THAT(relativeCentres, AreApproxEqual(expectedRelativeCentresAfterReflection_, 1e-12));
    }

    // Repeatedly applies reflection and checks how much the rotation matrix has drifted
    void test_rotation_matrix_error_cumulation(const ShapeTraits &traits, ReflectionSampler &reflectionSampler,
                                               const Shape &shape, const std::size_t numMoves,
                                               const double maxAllowedError2)
    {
        auto packing = construct_packing(traits, shape, {1, 1, 1});

        std::vector<std::size_t> particleIdxs{0};
        std::mt19937 mt(1234); // NOLINT(*-msc51-cpp)

        double maxObservedError2 = orientation_orthogonality_error2(packing[0].getOrientation());

        const auto &interaction = traits.getInteraction();
        for (std::size_t i{}; i < numMoves; i++) {
            const auto move = reflectionSampler.sampleMove(packing, particleIdxs, mt);
            packing.tryMove(move.particleIdx, move.translation, move.rotation, interaction);
            packing.acceptMove();

            const auto &orientation = packing[0].getOrientation();
            const double error2 = orientation_orthogonality_error2(orientation);
            REQUIRE(std::isfinite(error2));
            maxObservedError2 = std::max(maxObservedError2, error2);
        }

        CHECK(maxObservedError2 < maxAllowedError2);
    }
}

TEST_CASE("ReflectionSampler") {
    using trompeloeil::_;

    // An L-shaped trimer on an XZ plane, but with its geometric center (the middle ball) displaced from the origin
    const Vector<3> primaryAxis{0, 0, 1};
    const Vector<3> secondaryAxis{1, 0, 0};
    const Vector<3> geometricOrigin{1, 0, 1};
    const std::vector<Vector<3>> interactionCentres{
        geometricOrigin + Vector<3>{1, 0, 0},
        geometricOrigin + Vector<3>{0, 0, 0},
        geometricOrigin + Vector<3>{0, 0, 1}
    };

    MockShapeTraits traits;
    ALLOW_CALL(traits, hasHardPart()).RETURN(false);
    ALLOW_CALL(traits, hasSoftPart()).RETURN(false);
    ALLOW_CALL(traits, hasWallPart()).RETURN(false);
    ALLOW_CALL(traits, getRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getInteractionCentres()).RETURN(interactionCentres);
    ALLOW_CALL(traits, getPrimaryAxis(_)).RETURN(_1.getOrientation() * primaryAxis);
    ALLOW_CALL(traits, getSecondaryAxis(_)).RETURN(_1.getOrientation() * secondaryAxis);
    ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(_1.getOrientation() * geometricOrigin);

    const GeneralShapeAxis reflectionSymmetryAxis(ShapeGeometry::Axis::AUXILIARY);
    constexpr double every = 1;
    // The default XZ trimer is rotated by `pi/2` around `x`, so its `z` arm points along `+y`.
    const Shape rotatedShape({0.5, 0.5, 0.5}, Matrix<3, 3>::rotation(Vector<3>{1, 0, 0}, M_PI/2));

    SECTION("shape-local reflection axis") {
        // The shape-local reflection plane normal {1, -1, 0} rotates with the particle into lab {1, 0, -1}.
        ReflectionSampler reflectionSampler(GeneralShapeAxis(Vector<3>{1, -1, 0}), reflectionSymmetryAxis, every);
        reflectionSampler.setupForShapeTraits(traits);

        const std::vector<Vector<3>> expectedRelativeCentresAfterReflection{{0, 0, 1}, {0, 0, 0}, {0, -1, 0}};
        test_reflection_move(traits, reflectionSampler, rotatedShape, expectedRelativeCentresAfterReflection);
    }

    SECTION("lab reflection axis") {
        // The lab reflection plane normal stays fixed at {1, -1, 0} regardless of particle orientation.
        ReflectionSampler reflectionSampler(Vector<3>{1, -1, 0}, reflectionSymmetryAxis, every);
        reflectionSampler.setupForShapeTraits(traits);

        const std::vector<Vector<3>> expectedRelativeCentresAfterReflection{{0, 1, 0}, {0, 0, 0}, {-1, 0, 0}};
        test_reflection_move(traits, reflectionSampler, rotatedShape, expectedRelativeCentresAfterReflection);
    }
}

TEST_CASE("ReflectionSampler: orientation drift check")
{
    using trompeloeil::_;

    // Minimal ShapeTraits
    MockShapeTraits traits;
    ALLOW_CALL(traits, hasHardPart()).RETURN(false);
    ALLOW_CALL(traits, hasSoftPart()).RETURN(false);
    ALLOW_CALL(traits, hasWallPart()).RETURN(false);
    ALLOW_CALL(traits, getRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(Vector<3>{0, 0, 0});

    // Use "ugly" axes and initial shape orientation for a bigger chance of numerical instabilities

    const auto strangeReflectionAxis = Vector<3>{17, -29, 31}.normalized();
    const auto strangeSymmetryAxis = Vector<3>{-23, 37, 19}.normalized();
    constexpr double every = 1;
    ReflectionSampler reflectionSampler(GeneralShapeAxis(strangeReflectionAxis), GeneralShapeAxis(strangeSymmetryAxis),
                                        every);
    reflectionSampler.setupForShapeTraits(traits);

    const auto strangeInitialRotationAxis = Vector<3>{-11, 41, 37}.normalized();
    const Shape strangeOrientedShape({0, 0, 0}, Matrix<3, 3>::rotation(strangeInitialRotationAxis, 1.23456789));

    constexpr std::size_t numMoves = 100;
    constexpr double maxAllowedError2 = 1e-26;
    test_rotation_matrix_error_cumulation(traits, reflectionSampler, strangeOrientedShape, numMoves, maxAllowedError2);
}
