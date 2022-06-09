//
// Created by pkua on 09.06.22.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/lattice/LayerWiseCellOptimizationTransformer.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("LayerWiseCellOptimizationTransformer: 3 layers") {
    SphereTraits sphere(0.5);
    std::vector<Shape> initialShapes{Shape({0.1, 0.1, 0.15}), Shape({0.1, 0.5, 0.15}),
                                     Shape({0.1, 0.1, 0.5}), Shape({0.1, 0.5, 0.5}),
                                     Shape({0.1, 0.1, 0.8}), Shape({0.1, 0.5, 0.8})};
    TriclinicBox initialBox(5);
    Lattice lattice(UnitCell(initialBox, initialShapes), {3, 3, 3});
    LayerWiseCellOptimizationTransformer transformer(LatticeTraits::Axis::Z, sphere.getInteraction());

    transformer.transform(lattice);

    CHECK(lattice.isRegular());
    CHECK(lattice.isNormalized());
    const auto &initialBoxSides = initialBox.getSides();
    const auto &finalBoxSides = lattice.getCellBox().getSides();
    // Strictly equal, not approximately (x and y axes should not be altered)
    CHECK(finalBoxSides[0] == initialBoxSides[0]);
    CHECK(finalBoxSides[1] == initialBoxSides[1]);
    CHECK_THAT(finalBoxSides[2], IsApproxEqual(Vector<3>{0, 0, 3}, 1e-10));
    const auto &molecules = lattice.getUnitCellMolecules();
    REQUIRE(molecules.size() == 6);
    for (std::size_t i{}; i < 6; i++) {
        // Strictly equal, not approximately (x and y coordinates should not be altered)
        CHECK(molecules[i].getPosition()[0] == initialShapes[i].getPosition()[0]);
        CHECK(molecules[i].getPosition()[1] == initialShapes[i].getPosition()[1]);
    }
    CHECK(molecules[0].getPosition()[2] == Approx(1./6));
    CHECK(molecules[1].getPosition()[2] == Approx(1./6));
    CHECK(molecules[2].getPosition()[2] == Approx(3./6));
    CHECK(molecules[3].getPosition()[2] == Approx(3./6));
    CHECK(molecules[4].getPosition()[2] == Approx(5./6));
    CHECK(molecules[5].getPosition()[2] == Approx(5./6));
}