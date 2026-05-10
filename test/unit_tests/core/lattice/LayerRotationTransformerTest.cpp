//
// Created by pkua on 22.05.22.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"
#include "mocks/MockShapeTraits.h"

#include "core/lattice/LayerRotationTransformer.h"


namespace {
    constexpr bool isAlternating = true;
    constexpr bool isNonAlternating = false;
    constexpr bool isCumulative = true;
    constexpr bool isNonCumulative = false;

    void check_molecules(const std::vector<Shape> &molecules, const std::vector<Shape> &expectedMolecules) {
        REQUIRE(molecules.size() == expectedMolecules.size());

        for (std::size_t i = 0; i < molecules.size(); i++) {
            CHECK(molecules[i].getPosition() == expectedMolecules[i].getPosition());
            CHECK_THAT(molecules[i].getOrientation(), IsApproxEqual(expectedMolecules[i].getOrientation(), 1e-12));
        }
    }
}


TEST_CASE("LayerRotationTransformer: alternating") {
    const auto [isCumulativeMode, expectedEvenRotation, expectedOddRotation] = GENERATE(
        std::make_tuple(isNonCumulative, Matrix<3, 3>::rotation(M_PI/2, 0, 0), Matrix<3, 3>::rotation(-M_PI/2, 0, 0)),
        std::make_tuple(isCumulative, Matrix<3, 3>::identity(), Matrix<3, 3>::rotation(-M_PI/2, 0, 0))
    );

    DYNAMIC_SECTION((isCumulativeMode ? "cumulative" : "non-cumulative")) {
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
                LatticeTraits::Axis::Z, LatticeTraits::Axis::X, rotationAngle, isAlternating, isCumulativeMode
            );
            MockShapeTraits shapeTraits;

            layerRotationTransformer.transform(lattice, shapeTraits);

            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 2});
            check_molecules(lattice.getSpecificCell(0, 0, 0).getMolecules(), {
                Shape({0, 0.5, 0.25}, expectedEvenRotation),
                Shape({0.5, 0.5, 0.25}, expectedEvenRotation),
                Shape({0, 0.5, 0.75}, expectedOddRotation)
            });
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
                LatticeTraits::Axis::Z, LatticeTraits::Axis::X, rotationAngle, isNonAlternating, isNonCumulative
            );
            MockShapeTraits shapeTraits;

            layerRotationTransformer.transform(lattice, shapeTraits);

            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 2});
            const auto expectedRotation = Matrix<3, 3>::rotation(M_PI/2, 0, 0);
            check_molecules(lattice.getSpecificCell(0, 0, 0).getMolecules(), {
                Shape({0, 0.5, 0.25}, expectedRotation),
                Shape({0.5, 0.5, 0.25}, expectedRotation),
                Shape({0, 0.5, 0.75}, expectedRotation)
            });
        }
    }

    SECTION("cumulative") {
        SECTION("rotation angle in radians") {
            UnitCell unitCell(TriclinicBox(1),
                  {Shape({0.5, 0.5, 0.25}), Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 0.75})});
            Lattice lattice(unitCell, {2, 2, 2});
            LayerRotationTransformer layerRotationTransformer(
                LatticeTraits::Axis::Z, LatticeTraits::Axis::X, 2*M_PI/3, isNonAlternating, isCumulative
            );
            MockShapeTraits shapeTraits;

            layerRotationTransformer.transform(lattice, shapeTraits);

            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 1});
            check_molecules(lattice.getSpecificCell(0, 0, 0).getMolecules(), {
                Shape({0.5, 0.5, 0.125}, Matrix<3, 3>::identity()),
                Shape({0.5, 0.5, 0.25}, Matrix<3, 3>::rotation(2*M_PI/3, 0, 0)),
                Shape({0.5, 0.5, 0.375}, Matrix<3, 3>::rotation(4*M_PI/3, 0, 0)),
                Shape({0.5, 0.5, 0.625}, Matrix<3, 3>::identity()),
                Shape({0.5, 0.5, 0.75}, Matrix<3, 3>::rotation(2*M_PI/3, 0, 0)),
                Shape({0.5, 0.5, 0.875}, Matrix<3, 3>::rotation(4*M_PI/3, 0, 0))
            });
        }

        SECTION("rotation angle as quotient") {
            UnitCell unitCell(TriclinicBox(1),
                  {Shape({0.5, 0.5, 0.25}), Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 0.75})});
            Lattice lattice(unitCell, {2, 2, 2});
            using FullRotationQuotient = LayerRotationTransformer::FullRotationQuotient;
            LayerRotationTransformer layerRotationTransformer(LatticeTraits::Axis::Z, LatticeTraits::Axis::X,
                                                              FullRotationQuotient(1, 3), isNonAlternating,
                                                              isCumulative);
            MockShapeTraits shapeTraits;

            layerRotationTransformer.transform(lattice, shapeTraits);

            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 2});
            check_molecules(lattice.getSpecificCell(0, 0, 0).getMolecules(), {
                Shape({0.5, 0.5, 0.25}, Matrix<3, 3>::identity()),
                Shape({0.5, 0.5, 0.5}, Matrix<3, 3>::rotation(2*M_PI/3, 0, 0)),
                Shape({0.5, 0.5, 0.75}, Matrix<3, 3>::rotation(4*M_PI/3, 0, 0))
            });
        }
    }
}
