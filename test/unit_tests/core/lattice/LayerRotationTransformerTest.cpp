//
// Created by pkua on 22.05.22.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"
#include "mocks/MockShapeTraits.h"

#include "core/lattice/LayerRotationTransformer.h"


TEST_CASE("LayerRotationTransformer: alternating") {
    const auto [isCumulative, expectedEvenRotation, expectedOddRotation] = GENERATE(
        std::make_tuple(false, Matrix<3, 3>::rotation(M_PI/2, 0, 0), Matrix<3, 3>::rotation(-M_PI/2, 0, 0)),
        std::make_tuple(true, Matrix<3, 3>::identity(), Matrix<3, 3>::rotation(-M_PI/2, 0, 0))
    );

    DYNAMIC_SECTION((isCumulative ? "cumulative" : "non-cumulative")) {
        using RotationAngle = LayerRotationTransformer::RotationAngle;
        using FullRotationQuotient = LayerRotationTransformer::FullRotationQuotient;
        const auto [angleType, rotationAngle] = GENERATE(
            std::make_tuple("in radians", RotationAngle(M_PI/2)),
            std::make_tuple("as quotient", RotationAngle(FullRotationQuotient(1, 4)))
        );

        DYNAMIC_SECTION("rotation angle " << angleType) {
            UnitCell unitCell(TriclinicBox(1),
                              {Shape({0, 0.5, 0.25}), Shape({0.5, 0.5, 0.25}), Shape({0, 0.5, 0.75})});
            Lattice lattice(unitCell, {2, 2, 2});
            LayerRotationTransformer layerRotationTransformer(
                LatticeTraits::Axis::Z, LatticeTraits::Axis::X, rotationAngle, true, isCumulative
            );
            MockShapeTraits shapeTraits;

            layerRotationTransformer.transform(lattice, shapeTraits);

            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 2});
            const auto &molecules = lattice.getSpecificCell(0, 0, 0).getMolecules();
            REQUIRE(molecules.size() == 3);
            CHECK(molecules[0].getPosition() == Vector<3>{0, 0.5, 0.25});
            CHECK_THAT(molecules[0].getOrientation(), IsApproxEqual(expectedEvenRotation, 1e-12));
            CHECK(molecules[1].getPosition() == Vector<3>{0.5, 0.5, 0.25});
            CHECK_THAT(molecules[1].getOrientation(), IsApproxEqual(expectedEvenRotation, 1e-12));
            CHECK(molecules[2].getPosition() == Vector<3>{0, 0.5, 0.75});
            CHECK_THAT(molecules[2].getOrientation(), IsApproxEqual(expectedOddRotation, 1e-12));
        }
    }
}

TEST_CASE("LayerRotationTransformer: non-alternating") {
    SECTION("non-cumulative") {
        using RotationAngle = LayerRotationTransformer::RotationAngle;
        using FullRotationQuotient = LayerRotationTransformer::FullRotationQuotient;
        const auto [type, rotationAngle] = GENERATE(
            std::make_tuple("in radians", RotationAngle(M_PI/2)),
            std::make_tuple("as quotient", RotationAngle(FullRotationQuotient(1, 4)))
        );

        DYNAMIC_SECTION("rotation angle " << type) {
            UnitCell unitCell(TriclinicBox(1),
                              {Shape({0, 0.5, 0.25}), Shape({0.5, 0.5, 0.25}), Shape({0, 0.5, 0.75})});
            Lattice lattice(unitCell, {2, 2, 2});
            LayerRotationTransformer layerRotationTransformer(
                LatticeTraits::Axis::Z, LatticeTraits::Axis::X, rotationAngle, false
            );
            MockShapeTraits shapeTraits;

            layerRotationTransformer.transform(lattice, shapeTraits);

            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 2});
            const auto &molecules = lattice.getSpecificCell(0, 0, 0).getMolecules();
            REQUIRE(molecules.size() == 3);
            CHECK(molecules[0].getPosition() == Vector<3>{0, 0.5, 0.25});
            CHECK_THAT(molecules[0].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
            CHECK(molecules[1].getPosition() == Vector<3>{0.5, 0.5, 0.25});
            CHECK_THAT(molecules[1].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
            CHECK(molecules[2].getPosition() == Vector<3>{0, 0.5, 0.75});
            CHECK_THAT(molecules[2].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(M_PI/2, 0, 0), 1e-12));
        }
    }

    SECTION("cumulative") {
        SECTION("rotation angle in radians") {
            UnitCell unitCell(TriclinicBox(1),
                  {Shape({0.5, 0.5, 0.25}), Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 0.75})});
            Lattice lattice(unitCell, {2, 2, 2});
            LayerRotationTransformer layerRotationTransformer(
                LatticeTraits::Axis::Z, LatticeTraits::Axis::X, 2*M_PI/3, false, true
            );
            MockShapeTraits shapeTraits;

            layerRotationTransformer.transform(lattice, shapeTraits);

            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 1});
            const auto &molecules = lattice.getSpecificCell(0, 0, 0).getMolecules();
            REQUIRE(molecules.size() == 6);
            CHECK(molecules[0].getPosition() == Vector<3>{0.5, 0.5, 0.125});
            CHECK_THAT(molecules[0].getOrientation(), IsApproxEqual(Matrix<3, 3>::identity(), 1e-12));
            CHECK(molecules[1].getPosition() == Vector<3>{0.5, 0.5, 0.25});
            CHECK_THAT(molecules[1].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(2*M_PI/3, 0, 0), 1e-12));
            CHECK(molecules[2].getPosition() == Vector<3>{0.5, 0.5, 0.375});
            CHECK_THAT(molecules[2].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(4*M_PI/3, 0, 0), 1e-12));
            CHECK(molecules[3].getPosition() == Vector<3>{0.5, 0.5, 0.625});
            CHECK_THAT(molecules[3].getOrientation(), IsApproxEqual(Matrix<3, 3>::identity(), 1e-12));
            CHECK(molecules[4].getPosition() == Vector<3>{0.5, 0.5, 0.75});
            CHECK_THAT(molecules[4].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(2*M_PI/3, 0, 0), 1e-12));
            CHECK(molecules[5].getPosition() == Vector<3>{0.5, 0.5, 0.875});
            CHECK_THAT(molecules[5].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(4*M_PI/3, 0, 0), 1e-12));
        }

        SECTION("rotation angle as quotient") {
            UnitCell unitCell(TriclinicBox(1),
                  {Shape({0.5, 0.5, 0.25}), Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 0.75})});
            Lattice lattice(unitCell, {2, 2, 2});
            using FullRotationQuotient = LayerRotationTransformer::FullRotationQuotient;
            LayerRotationTransformer layerRotationTransformer(
                LatticeTraits::Axis::Z, LatticeTraits::Axis::X, FullRotationQuotient(1, 3), false, true
            );
            MockShapeTraits shapeTraits;

            layerRotationTransformer.transform(lattice, shapeTraits);

            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 2});
            const auto &molecules = lattice.getSpecificCell(0, 0, 0).getMolecules();
            REQUIRE(molecules.size() == 3);
            CHECK(molecules[0].getPosition() == Vector<3>{0.5, 0.5, 0.25});
            CHECK_THAT(molecules[0].getOrientation(), IsApproxEqual(Matrix<3, 3>::identity(), 1e-12));
            CHECK(molecules[1].getPosition() == Vector<3>{0.5, 0.5, 0.5});
            CHECK_THAT(molecules[1].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(2*M_PI/3, 0, 0), 1e-12));
            CHECK(molecules[2].getPosition() == Vector<3>{0.5, 0.5, 0.75});
            CHECK_THAT(molecules[2].getOrientation(), IsApproxEqual(Matrix<3, 3>::rotation(4*M_PI/3, 0, 0), 1e-12));
        }
    }
}
