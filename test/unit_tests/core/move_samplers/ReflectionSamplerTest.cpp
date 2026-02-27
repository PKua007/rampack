//
// Created by Michal Ciesla on 14.01.25.
//

#include <numeric>
#include <algorithm>

#include <catch2/catch.hpp>

#include "matchers/InteractionCentresApproxMatcher.h"

#include "mocks/MockShapeTraits.h"

#include "core/lattice/Lattice.h"
#include "core/move_samplers/ReflectionSampler.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    Packing construct_packing(const ShapeTraits &traits)
    {
        Lattice lattice(UnitCell(TriclinicBox(5), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
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

    // Builds a small packing, applies one sampled reflection move, and verifies that the reflection was correctly
    // applied by inspecting interaction centers after reflection.
    void test_reflection_move(const ShapeTraits &traits, ReflectionSampler &reflectionSampler,
                              const std::vector<Vector<3>> &expectedRelativeCentresAfterReflection_)
    {
        const auto &interaction = traits.getInteraction();
        const auto &geometry = traits.getGeometry();

        auto packing = construct_packing(traits);

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

    // Reflect through the reflection plane spanned by the Z axis and the disector of the angle between X and Y axes
    const GeneralizedShapeAxis reflectionAxis = Vector<3>{1, -1, 0};
    const GeneralizedShapeAxis reflectionSymmetryAxis = ShapeGeometry::Axis::AUXILIARY;
    constexpr double every = 1;
    ReflectionSampler reflectionSampler(reflectionAxis, reflectionSymmetryAxis, every);
    reflectionSampler.setupForShapeTraits(traits);

    const std::vector<Vector<3>> expectedRelativeCentresAfterReflection{{0, 1, 0}, {0, 0, 0}, {0, 0, 1}};
    test_reflection_move(traits, reflectionSampler, expectedRelativeCentresAfterReflection);
}
