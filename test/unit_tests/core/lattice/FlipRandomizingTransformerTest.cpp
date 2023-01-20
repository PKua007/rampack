//
// Created by pkua on 25.05.22.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"
#include "matchers/VectorApproxMatcher.h"

#include "mocks/MockShapeTraits.h"

#include "core/lattice/FlipRandomizingTransformer.h"


TEST_CASE("FlipRandomizingTransformer") {
    using trompeloeil::_;

    Lattice lattice(UnitCell(TriclinicBox(4), {Shape({0.5, 0.5, 0.5})}), {4, 4, 4});
    MockShapeTraits traits;
    ALLOW_CALL(traits, getSecondaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{0, 0, 1});
    ALLOW_CALL(traits, getGeometricOrigin(_)).RETURN(_1.getOrientation() * Vector<3>{0.1, 0.1, 0.1});
    FlipRandomizingTransformer flipRandomizingTransformer(1234);

    flipRandomizingTransformer.transform(lattice, traits);

    const auto &dim = lattice.getDimensions();
    for (std::size_t i{}; i < dim[0]; i++) {
        for (std::size_t j{}; j < dim[1]; j++) {
            for (std::size_t k{}; k < dim[2]; k++) {
                const auto &molecules = lattice.getSpecificCellMolecules(i, j, k);
                REQUIRE(molecules.size() == 1);
                const auto &molecule = molecules.front();
                Matrix<3, 3> notRotated = Matrix<3, 3>::identity();
                Matrix<3, 3> rotated{-1,  0, 0,
                                      0, -1, 0,
                                      0,  0, 1};
                INFO("Testing cell (" << i << ", " << j << ", " << k << ")...");
                CHECK_THAT(molecule.getOrientation(), IsApproxEqual(rotated, 1e-12) || IsApproxEqual(notRotated, 1e-12));
                if (molecule.getOrientation() == notRotated)
                    CHECK_THAT(molecule.getPosition(), IsApproxEqual(Vector<3>{0.5, 0.5, 0.5}, 1e-12));
                else    // In absolute coords translation due to displaced origin is 0.2, so 0.05 in relative
                    CHECK_THAT(molecule.getPosition(), IsApproxEqual(Vector<3>{0.55, 0.55, 0.5}, 1e-12));
            }
        }
    }
}