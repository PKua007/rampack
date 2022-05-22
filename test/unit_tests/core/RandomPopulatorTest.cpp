//
// Created by pkua on 22.05.22.
//

#include <iterator>
#include <algorithm>

#include <catch2/catch.hpp>

#include "core/lattice/RandomPopulator.h"


TEST_CASE("RandomPopulator") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 0.25})}), {2, 2, 2});
    RandomPopulator randomPopulator(1234);

    auto shapes = randomPopulator.populateLattice(lattice, 8);

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