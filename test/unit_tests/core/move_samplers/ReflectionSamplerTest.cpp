//
// Created by Michal Ciesla on 14.01.25.
//

#include <algorithm>
#include <numeric>

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeTraits.h"

#include "core/lattice/Lattice.h"
#include "core/move_samplers/ReflectionSampler.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    bool containsApproxAndMark(const std::vector<Vector<3>> &vectors, const Vector<3> &target,
                               std::vector<bool> &used, double epsilon)
    {
        const double epsilon2 = epsilon * epsilon;
        for (std::size_t i = 0; i < vectors.size(); i++) {
            if (used[i])
                continue;

            if ((vectors[i] - target).norm2() < epsilon2) {
                used[i] = true;
                return true;
            }
        }

        return false;
    }

    bool areSameSetApprox(const std::vector<Vector<3>> &actual, const std::vector<Vector<3>> &expected, double epsilon)
    {
        if (actual.size() != expected.size())
            return false;

        std::vector<bool> used(actual.size(), false);
        for (const auto &expectedVector : expected) {
            if (!containsApproxAndMark(actual, expectedVector, used, epsilon))
                return false;
        }

        return true;
    }
}


TEST_CASE("ReflectionSampler") {
    using trompeloeil::_;

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

    Lattice lattice(UnitCell(TriclinicBox(2), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), traits.getInteraction());

    ReflectionSampler reflectionSampler(GeneralizedShapeAxis(Vector<3>{1, -1, 0}),
                                        FlipAxis(ShapeGeometry::Axis::AUXILIARY), 1);
    reflectionSampler.setupForShapeTraits(traits);

    std::vector<std::size_t> particleIdxs(packing.size());
    std::iota(particleIdxs.begin(), particleIdxs.end(), 0);
    std::mt19937 mt(1234); // NOLINT(*-msc51-cpp)

    auto move = reflectionSampler.sampleMove(packing, particleIdxs, mt);

    REQUIRE(move.particleIdx < packing.size());
    REQUIRE(move.moveType == MoveSampler::MoveType::ROTOTRANSLATION);

    const auto &interaction = traits.getInteraction();
    const auto &geometry = traits.getGeometry();
    packing.tryMove(move.particleIdx, move.translation, move.rotation, interaction);
    packing.acceptMove();

    const Shape &movedShape = packing[move.particleIdx];
    const auto reflectedShapeInteractionCentres = interaction.getInteractionCentresForShape(movedShape);
    const auto reflectedShapeGeometricOrigin = movedShape.getPosition() + geometry.getGeometricOrigin(movedShape);

    std::vector<Vector<3>> relativeCentres;
    relativeCentres.reserve(reflectedShapeInteractionCentres.size());
    for (const auto &centre : reflectedShapeInteractionCentres)
        relativeCentres.push_back(centre - reflectedShapeGeometricOrigin);

    const std::vector<Vector<3>> expectedRelativeCentres{{0, 1, 0}, {0, 0, 0}, {0, 0, 1}};
    CHECK(areSameSetApprox(relativeCentres, expectedRelativeCentres, 1e-12));
}
