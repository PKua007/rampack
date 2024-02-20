//
// Created by pkua on 22.05.22.
//

#include <catch2/catch.hpp>

#include "core/lattice/SerialPopulator.h"


TEST_CASE("SerialPopulator: single shape in cell") {
    Matrix<3, 3> noRot = Matrix<3, 3>::identity();
    int data = 3;
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.5}, noRot, data)}), {2, 2, 2});
    SerialPopulator serialPopulator("yxz");

    auto shapes = serialPopulator.populateLattice(lattice, 3);

    CHECK(shapes == std::vector<Shape>{
        Shape({0.5, 0.5, 0.5}, noRot, data),
        Shape({0.5, 0.5, 1.5}, noRot, data),
        Shape({1.5, 0.5, 0.5}, noRot, data)
    });
}


TEST_CASE("SerialPopulator: 2 shapes in cell") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 0.25})}), {2, 2, 2});
    SerialPopulator serialPopulator("yxz");

    auto shapes = serialPopulator.populateLattice(lattice, 5);

    CHECK(shapes == std::vector<Shape>{Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 0.25}),
                                       Shape({0.5, 0.5, 1.5}), Shape({0.5, 0.5, 1.25}),
                                       Shape({1.5, 0.5, 0.5})});
}