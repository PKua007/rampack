//
// Created by pkua on 09.06.22.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/lattice/LayerWiseCellOptimizationTransformer.h"
#include "core/shapes/PolysphereTraits.h"


TEST_CASE("LayerWiseCellOptimizationTransformer: 3 layers") {
    // Molecule:
    //   #####
    // ##  _  ##    <- larger ball with mass centre
    // ##     ##
    //   #####
    //    ###       <- 2x smaller, tangent ball
    //    ###
    PolysphereTraits::PolysphereGeometry geometry({{{0, 0, 0}, 2}, {{0, 0, -3}, 1}},
                                                  {0, 0, -1}, {0, 1, 0}, {0, 0, -1});
    PolysphereTraits traits(std::move(geometry));
    // Unit cell:
    //   ####    ####
    //   ####    ####
    //    ##      ##
    //
    //    ##      ##
    //   ####    ####
    //   ####    ####
    //
    //   ####    ####
    //   ####    ####
    //    ##      ##
    auto rotated = Matrix<3, 3>::rotation(0, M_PI, 0);
    std::vector<Shape> initialShapes{Shape({0.1, 0.1, 0.15}), Shape({0.1, 0.5, 0.15}),
                                     Shape({0.1, 0.1, 0.5}, rotated), Shape({0.1, 0.5, 0.5}, rotated),
                                     Shape({0.1, 0.1, 0.8}), Shape({0.1, 0.5, 0.8})};
    TriclinicBox initialBox(30);
    Lattice lattice(UnitCell(initialBox, initialShapes), {3, 3, 2});
    double spacing = 2;
    LayerWiseCellOptimizationTransformer transformer(traits.getInteraction(), LatticeTraits::Axis::Z, spacing);

    transformer.transform(lattice);

    CHECK(lattice.isRegular());
    CHECK(lattice.isNormalized());
    const auto &initialBoxSides = initialBox.getSides();
    const auto &finalBoxSides = lattice.getCellBox().getSides();
    // Strictly equal, not approximately (x and y axes should not be altered)
    CHECK(finalBoxSides[0] == initialBoxSides[0]);
    CHECK(finalBoxSides[1] == initialBoxSides[1]);
    CHECK_THAT(finalBoxSides[2], IsApproxEqual(Vector<3>{0, 0, 24}, 1e-10));
    const auto &molecules = lattice.getUnitCellMolecules();
    REQUIRE(molecules.size() == 6);
    for (std::size_t i{}; i < 6; i++) {
        // Strictly equal, not approximately (x and y coordinates should not be altered)
        CHECK(molecules[i].getPosition()[0] == initialShapes[i].getPosition()[0]);
        CHECK(molecules[i].getPosition()[1] == initialShapes[i].getPosition()[1]);
    }
    CHECK(molecules[0].getPosition()[2] == Approx(2./12));
    CHECK(molecules[1].getPosition()[2] == Approx(2./12));
    CHECK(molecules[2].getPosition()[2] == Approx(5./12));
    CHECK(molecules[3].getPosition()[2] == Approx(5./12));
    CHECK(molecules[4].getPosition()[2] == Approx(10./12));
    CHECK(molecules[5].getPosition()[2] == Approx(10./12));
}

TEST_CASE("LayerWiseCellOptimizationTransformer: 1 layer") {
    // This test also tests for the bug where the maximally shrunk end of cell size bisection was incorrect due to
    // wrong calculation of a shrinking factor - it happened when a range of interaction was bigger than optimal
    // distance and initial cell was already small (but not optimal)

    // Dimer, but mass centre in a middle of a monomer => total range = 3
    PolysphereTraits::PolysphereGeometry geometry({{{0, 0, 0}, 0.5}, {{0, 0, 1}, 0.5}},
                                                  {0, 0, 1}, {0, 1, 0}, {0, 0, 0});
    PolysphereTraits traits(std::move(geometry));
    TriclinicBox initialBox(std::array<double, 3>{5, 5, 3});
    Lattice lattice(UnitCell(initialBox, {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
    double spacing = 0.5;
    LayerWiseCellOptimizationTransformer transformer(traits.getInteraction(), LatticeTraits::Axis::Z, spacing);

    transformer.transform(lattice);

    CHECK(lattice.isRegular());
    CHECK(lattice.isNormalized());
    const auto &initialBoxSides = initialBox.getSides();
    const auto &finalBoxSides = lattice.getCellBox().getSides();
    // Strictly equal, not approximately (x and y axes should not be altered)
    CHECK(finalBoxSides[0] == initialBoxSides[0]);
    CHECK(finalBoxSides[1] == initialBoxSides[1]);
    CHECK_THAT(finalBoxSides[2], IsApproxEqual(Vector<3>{0, 0, 2.5}, 1e-10));
    const auto &molecules = lattice.getUnitCellMolecules();
    REQUIRE(molecules.size() == 1);
    CHECK(molecules[0].getPosition()[0] == 0.5);
    CHECK(molecules[0].getPosition()[1] == 0.5);
    CHECK(molecules[0].getPosition()[2] == Approx(0.5));
}