//
// Created by pkua on 22.05.22.
//

#include <iterator>
#include <algorithm>

#include <catch2/catch.hpp>

#include "core/lattice/RandomPopulator.h"


TEST_CASE("RandomPopulator") {
    Matrix<3, 3> rot = Matrix<3, 3>::rotation(M_PI, 0, 0);
    int data = 3;
    UnitCell cell(TriclinicBox(1), {Shape({0.5, 0.5, 0.5}, rot, data), Shape({0.5, 0.5, 0.25}, rot, data)});
    Lattice lattice(cell, {2, 2, 2});
    RandomPopulator randomPopulator(1234);

    auto shapes = randomPopulator.populateLattice(lattice, 8);
    for (const auto &shape : shapes) {
        CHECK(shape.getOrientation() == rot);
        CHECK(shape.getData() == ShapeData(data));
    }

    CHECK(shapes.size() == 8);
    auto allShapes = lattice.generateMolecules();
    auto getShapeIndex = [allShapes](const Shape &shape) {
        auto it = std::find(allShapes.begin(), allShapes.end(), shape);
        REQUIRE(it != allShapes.end());
        return it - allShapes.begin();
    };
    std::vector<std::size_t> indices;
    std::transform(shapes.begin(), shapes.end(), std::back_inserter(indices), getShapeIndex);
    CHECK(std::is_sorted(indices.begin(), indices.end()));
    CHECK(std::unique(indices.begin(), indices.end()) == indices.end());
}