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


TEST_CASE("FlipSampler") {
    using trompeloeil::_;

    Lattice lattice(UnitCell(TriclinicBox(2), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
    MockShapeTraits traits;
    ALLOW_CALL(traits, hasHardPart()).RETURN(true);
    ALLOW_CALL(traits, hasSoftPart()).RETURN(false);
    ALLOW_CALL(traits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, getTotalRangeRadius()).RETURN(1);
    ALLOW_CALL(traits, overlapBetween(_, _, _, _, _, _, _)).RETURN(_7.getDistance2(_1, _4) < 1);
    ALLOW_CALL(traits, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
    ALLOW_CALL(traits, getSecondaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{0, 0, 1});
    ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(_1.getOrientation() * Vector<3>{0.1, 0.1, 0.1});
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(lattice.getLatticeBox(), lattice.generateMolecules(), std::move(pbc), traits.getInteraction());
    FlipSampler flipSampler(1);
    std::vector<std::size_t> particleIdxs(packing.size());
    std::iota(particleIdxs.begin(), particleIdxs.end(), 0);
    std::mt19937 mt(1234);

    auto move = flipSampler.sampleMove(packing, traits, particleIdxs, mt);

    CHECK(move.particleIdx < packing.size());
    CHECK(move.moveType == MoveSampler::MoveType::ROTOTRANSLATION);
    CHECK_THAT(move.rotation, IsApproxEqual(Matrix<3, 3>{-1,  0, 0,
                                                          0, -1, 0,
                                                          0,  0, 1},
                                            1e-12));
    CHECK_THAT(move.translation, IsApproxEqual(Vector<3>{0.2, 0.2, 0}, 1e-12));
}
