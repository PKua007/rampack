//
// Created by pkua on 22.05.22.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"

#include "core/lattice/LayerRotationTransformer.h"


TEST_CASE("LayerRotationTransformer: alternating") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0, 0.5, 0.25}), Shape({0.5, 0.5, 0.25}), Shape({0, 0.5, 0.75})}),
                    {2, 2, 2});
    LayerRotationTransformer layerRotationTransformer(LatticeTraits::Axis::Z, LatticeTraits::Axis::X, M_PI/2, true);

    layerRotationTransformer.transform(lattice);

    const auto &molecules = lattice.getCell(0, 0, 0).getMolecules();
    CHECK(molecules[0].getPosition() == Vector<3>{0, 0.5, 0.25});
    CHECK_THAT(molecules[0].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
    CHECK(molecules[1].getPosition() == Vector<3>{0.5, 0.5, 0.25});
    CHECK_THAT(molecules[1].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
    CHECK(molecules[2].getPosition() == Vector<3>{0, 0.5, 0.75});
    CHECK_THAT(molecules[2].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(-M_PI/2, 0, 0), 1e-12));
}

TEST_CASE("LayerRotationTransformer: non-alternating") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0, 0.5, 0.25}), Shape({0.5, 0.5, 0.25}), Shape({0, 0.5, 0.75})}),
                    {2, 2, 2});
    LayerRotationTransformer layerRotationTransformer(LatticeTraits::Axis::Z, LatticeTraits::Axis::X, M_PI/2, false);

    layerRotationTransformer.transform(lattice);

    const auto &molecules = lattice.getCell(0, 0, 0).getMolecules();
    CHECK(molecules[0].getPosition() == Vector<3>{0, 0.5, 0.25});
    CHECK_THAT(molecules[0].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
    CHECK(molecules[1].getPosition() == Vector<3>{0.5, 0.5, 0.25});
    CHECK_THAT(molecules[1].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
    CHECK(molecules[2].getPosition() == Vector<3>{0, 0.5, 0.75});
    CHECK_THAT(molecules[2].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
}