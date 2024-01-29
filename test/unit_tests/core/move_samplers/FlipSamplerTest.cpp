//
// Created by pkua on 25.05.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"

#include "matchers/MatrixApproxMatcher.h"
#include "matchers/VectorApproxMatcher.h"

#include "core/move_samplers/FlipSampler.h"

#include "core/lattice/Lattice.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    void test_flip_move(const ShapeTraits &traits, MoveSampler::MoveType expectedMoveType,
                        const Matrix<3, 3> &expectedRotation, const Vector<3> &expectedTranslation)
    {
        Lattice lattice(UnitCell(TriclinicBox(2), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();
        Packing packing(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), traits.getInteraction());
        FlipSampler flipSampler(1);
        flipSampler.setupForShapeTraits(traits);
        std::vector<std::size_t> particleIdxs(packing.size());
        std::iota(particleIdxs.begin(), particleIdxs.end(), 0);
        std::mt19937 mt(1234);

        auto move = flipSampler.sampleMove(packing, particleIdxs, mt);

        CHECK(move.particleIdx < packing.size());
        CHECK(move.moveType == expectedMoveType);
        CHECK_THAT(move.rotation, IsApproxEqual(expectedRotation, 1e-12));
        CHECK_THAT(move.translation, IsApproxEqual(expectedTranslation, 1e-12));
    }
}


TEST_CASE("FlipSampler") {
    using trompeloeil::_;

    MockShapeTraits traits;
    ALLOW_CALL(traits, hasHardPart()).RETURN(true);
    ALLOW_CALL(traits, hasSoftPart()).RETURN(false);
    ALLOW_CALL(traits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, overlapBetween(_, _, _, _, _, _, _, _, _)).RETURN(_9.getDistance2(_1, _5) < 1);
    ALLOW_CALL(traits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});

    SECTION("primary + secondary + nonzero origin") {
        ALLOW_CALL(traits, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{0, 1, 0});
        ALLOW_CALL(traits, getSecondaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{0, 0, 1});
        ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(_1.getOrientation() * Vector<3>{0.1, 0.1, 0.1});

        auto expectedMoveType = MoveSampler::MoveType::ROTOTRANSLATION;
        Matrix<3, 3> expectedRotation{-1,  0, 0,
                                       0, -1, 0,
                                       0,  0, 1};
        Vector<3> expectedTranslation{0.2, 0.2, 0};
        test_flip_move(traits, expectedMoveType, expectedRotation, expectedTranslation);
    }

    SECTION("primary + zero origin") {
        ALLOW_CALL(traits, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{2./3, -2./3, 1./3});
        ALLOW_CALL(traits, getSecondaryAxis(_)).THROW(std::runtime_error("no secondary axis"));
        ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(Vector<3>{0, 0, 0});

        auto expectedMoveType = MoveSampler::MoveType::ROTATION;
        Matrix<3, 3> expectedRotation{0, 1,  0,
                                      1, 0,  0,
                                      0, 0, -1};
        Vector<3> expectedTranslation{0, 0, 0};
        test_flip_move(traits, expectedMoveType, expectedRotation, expectedTranslation);
    }
}
