//
// Created by pkua on 22.05.22.
//

#include <catch2/catch.hpp>

#include "core/lattice/ColumnarTransformer.h"


TEST_CASE("ColumnarTransformer") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0, 0.5, 0.25}), Shape({0, 0.5, 0.75}), Shape({0.5, 0.5, 0.25})}),
                    {2, 2, 2});
    ColumnarTransformer transformer(LatticeTraits::Axis::Z, 123456);

    transformer.transform(lattice);

    CHECK_FALSE(lattice.isRegular());
    const auto &cell000 = lattice.getCell(0, 0, 0);
    const auto &cell001 = lattice.getCell(0, 0, 1);
    const auto &cell110 = lattice.getCell(1, 1, 0);
    double intraCellColDistance = cell000[1].getPosition()[2] - cell000[0].getPosition()[2];
    CHECK((intraCellColDistance == Approx(0.5) || intraCellColDistance == Approx(-0.5)));
    CHECK(cell000[0].getPosition()[0] == 0);
    CHECK(cell000[0].getPosition()[1] == 0.5);
    CHECK(cell000[1].getPosition()[0] == 0);
    CHECK(cell000[1].getPosition()[1] == 0.5);
    CHECK(cell000[2].getPosition()[0] == 0.5);
    CHECK(cell000[2].getPosition()[1] == 0.5);
    double interCellColDistance = cell001[0].getPosition()[2] - cell000[0].getPosition()[2];
    CHECK(interCellColDistance == Approx(0));
    double differentColsDistance1 = cell000[2].getPosition()[2] - cell000[0].getPosition()[2];
    CHECK(differentColsDistance1 != Approx(0));
    double differentColsDistance2 = cell110[0].getPosition()[2] - cell000[0].getPosition()[2];
    CHECK(differentColsDistance2 != Approx(0));
}