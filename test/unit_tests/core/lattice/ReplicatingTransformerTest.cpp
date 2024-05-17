//
// Created by Piotr Kubala on 17/05/2024.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"

#include "core/lattice/ReplicatingTransformer.h"


TEST_CASE("ReplicatingTransformer: construction exceptions") {
    SECTION("zero scale") {
        CHECK_THROWS(ReplicatingTransformer({2, 2, 0}));
    }

    SECTION("identity") {
        CHECK_THROWS(ReplicatingTransformer({1, 1, 1}));
    }
}

TEST_CASE("ReplicatingTransformer: regular lattice") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.5})}),
                    {2, 2, 2});
    ReplicatingTransformer replicatingTransformer({1, 2, 3});
    MockShapeTraits shapeTraits;

    replicatingTransformer.transform(lattice, shapeTraits);

    REQUIRE(lattice.isRegular());
    CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 4, 6});
    CHECK(lattice.getCellBox() == TriclinicBox(1));
    CHECK(lattice.getUnitCellMolecules() == std::vector<Shape>{Shape({0.5, 0.5, 0.5})});
}

TEST_CASE("ReplicatingTransformer: irregular lattice") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.4})}),
                    {1, 1, 2});
    lattice.modifySpecificCellMolecules(0, 0, 1).front().setPosition({0.5, 0.5, 0.6});
    ReplicatingTransformer replicatingTransformer({1, 2, 3});
    MockShapeTraits shapeTraits;

    replicatingTransformer.transform(lattice, shapeTraits);

    REQUIRE(lattice.isRegular());
    CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{1, 2, 3});
    CHECK(lattice.getCellBox() == TriclinicBox(std::array<double, 3>{1, 1, 2}));
    CHECK(lattice.getUnitCellMolecules() == std::vector<Shape>{Shape({0.5, 0.5, 0.2}), Shape({0.5, 0.5, 0.8})});
}