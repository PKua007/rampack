//
// Created by pkua on 22.05.22.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"
#include "mocks/MockShapeTraits.h"

#include "core/lattice/LayerRotationTransformer.h"


TEST_CASE("LayerRotationTransformer: alternating") {
    using RotationAngle = LayerRotationTransformer::RotationAngle;
    using FullRotationQuotient = LayerRotationTransformer::FullRotationQuotient;
    const auto [type, rotationAngle] = GENERATE(
        std::make_tuple("in radians", RotationAngle(M_PI/2)),
        std::make_tuple("as quotient", RotationAngle(FullRotationQuotient(1, 4)))
    );

    DYNAMIC_SECTION("rotation angle " << type) {
        UnitCell unitCell(TriclinicBox(1), {Shape({0, 0.5, 0.25}), Shape({0.5, 0.5, 0.25}), Shape({0, 0.5, 0.75})});
        Lattice lattice(unitCell, {2, 2, 2});
        LayerRotationTransformer layerRotationTransformer(
            LatticeTraits::Axis::Z, LatticeTraits::Axis::X, rotationAngle, true
        );
        MockShapeTraits shapeTraits;

        layerRotationTransformer.transform(lattice, shapeTraits);

        const auto &molecules = lattice.getSpecificCell(0, 0, 0).getMolecules();
        CHECK(molecules[0].getPosition() == Vector<3>{0, 0.5, 0.25});
        CHECK_THAT(molecules[0].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
        CHECK(molecules[1].getPosition() == Vector<3>{0.5, 0.5, 0.25});
        CHECK_THAT(molecules[1].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
        CHECK(molecules[2].getPosition() == Vector<3>{0, 0.5, 0.75});
        CHECK_THAT(molecules[2].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(-M_PI/2, 0, 0), 1e-12));
    }
}

TEST_CASE("LayerRotationTransformer: non-alternating") {
    using RotationAngle = LayerRotationTransformer::RotationAngle;
    using FullRotationQuotient = LayerRotationTransformer::FullRotationQuotient;
    const auto [type, rotationAngle] = GENERATE(
        std::make_tuple("in radians", RotationAngle(M_PI/2)),
        std::make_tuple("as quotient", RotationAngle(FullRotationQuotient(1, 4)))
    );

    DYNAMIC_SECTION("rotation angle " << type) {
        UnitCell unitCell(TriclinicBox(1), {Shape({0, 0.5, 0.25}), Shape({0.5, 0.5, 0.25}), Shape({0, 0.5, 0.75})});
        Lattice lattice(unitCell, {2, 2, 2});
        LayerRotationTransformer layerRotationTransformer(
            LatticeTraits::Axis::Z, LatticeTraits::Axis::X, rotationAngle, false
        );
        MockShapeTraits shapeTraits;

        layerRotationTransformer.transform(lattice, shapeTraits);

        const auto &molecules = lattice.getSpecificCell(0, 0, 0).getMolecules();
        CHECK(molecules[0].getPosition() == Vector<3>{0, 0.5, 0.25});
        CHECK_THAT(molecules[0].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
        CHECK(molecules[1].getPosition() == Vector<3>{0.5, 0.5, 0.25});
        CHECK_THAT(molecules[1].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
        CHECK(molecules[2].getPosition() == Vector<3>{0, 0.5, 0.75});
        CHECK_THAT(molecules[2].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
    }
}