//
// Created by pkua on 23.05.22.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"

#include "core/lattice/CellOptimizationTransformer.h"

#include "core/shapes/SphereTraits.h"


TEST_CASE("CellOptimizationTransformer: orthorhombic lattice") {
    UnitCell unitCell(TriclinicBox(std::array<double, 3>{5, 5, 5}), {Shape({0.5, 0.5, 0.5})});
    Lattice lattice(unitCell, {4, 4, 4});
    auto shapes = lattice.generateMolecules();
    SphereTraits sphereTraits(0.5);
    CellOptimizationTransformer cellOptimizationTransformer("xyz", {0.1, 0.2, 0.3});

    cellOptimizationTransformer.transform(lattice, sphereTraits);

    CHECK(lattice.isRegular());
    Matrix<3, 3> expectedMatrix{1.1, 0, 0, 0, 1.2, 0, 0, 0, 1.3};
    CHECK_THAT(lattice.getCellBox().getDimensions(), IsApproxEqual(expectedMatrix, 1e-12));
}

TEST_CASE("CellOptimizationTransformer: hexagonal lattice") {
    double height = std::sqrt(3.)/2;
    Matrix<3, 3> unitCellBox{1,    0.5, 0,
                             0, height, 0,
                             0,      0, 1};
    UnitCell unitCell(TriclinicBox(5. * unitCellBox), {Shape({0.5, 0.5, 0.5})});
    Lattice lattice(unitCell, {4, 4, 4});
    auto shapes = lattice.generateMolecules();
    SphereTraits sphereTraits(0.5);
    CellOptimizationTransformer cellOptimizationTransformer("xyz", {0.1, 0.2, 0.3});

    cellOptimizationTransformer.transform(lattice, sphereTraits);

    CHECK(lattice.isRegular());
    Matrix<3, 3> expectedMatrix{(height + 0.1)/height, 0.5*(height + 0.2)/height,   0,
                                                    0,              height + 0.2,   0,
                                                    0,                         0, 1.3};
    CHECK_THAT(lattice.getCellBox().getDimensions(), IsApproxEqual(expectedMatrix, 1e-12));
}