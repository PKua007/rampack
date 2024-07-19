//
// Created by Piotr Kubala on 17/05/2024.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"

#include "core/lattice/transformers/ReplicatingTransformer.h"


TEST_CASE("ReplicatingTransformer: construction exceptions") {
    SECTION("zero scale") {
        CHECK_THROWS(ReplicatingTransformer({2, 2, 0}));
    }

    SECTION("identity") {
        CHECK_THROWS(ReplicatingTransformer({1, 1, 1}));
    }
}

TEST_CASE("ReplicatingTransformer: as unit cell") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.4})}), {1, 1, 2});
    lattice.modifySpecificCellMolecules(0, 0, 1).front().setPosition({0.5, 0.5, 0.6});
    MockShapeTraits shapeTraits;
    ReplicatingTransformer replicatingTransformer({1, 2, 3}, true);

    replicatingTransformer.transform(lattice, shapeTraits);

    REQUIRE(lattice.isRegular());
    CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{1, 2, 3});
    CHECK(lattice.getCellBox() == TriclinicBox(std::array<double, 3>{1, 1, 2}));
    CHECK(lattice.getUnitCellMolecules() == std::vector<Shape>{Shape({0.5, 0.5, 0.2}), Shape({0.5, 0.5, 0.8})});
}

TEST_CASE("ReplicatingTransformer: preserving cells") {
    MockShapeTraits shapeTraits;

    SECTION("regular lattice") {
        Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});
        ReplicatingTransformer replicatingTransformer({1, 2, 3});

        replicatingTransformer.transform(lattice, shapeTraits);

        REQUIRE(lattice.isRegular());
        CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 4, 6});
        CHECK(lattice.getCellBox() == TriclinicBox(1));
        CHECK(lattice.getUnitCellMolecules() == std::vector<Shape>{Shape({0.5, 0.5, 0.5})});
    }

    SECTION("irregular lattice") {
        Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.4})}), {1, 1, 2});
        lattice.modifySpecificCellMolecules(0, 0, 1).front().setPosition({0.5, 0.5, 0.6});
        ReplicatingTransformer replicatingTransformer({1, 2, 3}, false);

        replicatingTransformer.transform(lattice, shapeTraits);

        REQUIRE_FALSE(lattice.isRegular());
        CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{1, 2, 6});
        CHECK(lattice.getCellBox() == TriclinicBox(std::array<double, 3>{1, 1, 1}));
        std::vector<Shape> cell000{Shape({0.5, 0.5, 0.4})};
        std::vector<Shape> cell001{Shape({0.5, 0.5, 0.6})};
        CHECK(lattice.getSpecificCellMolecules(0, 0, 0) == cell000);    // original
        CHECK(lattice.getSpecificCellMolecules(0, 1, 0) == cell000);    // replica
        CHECK(lattice.getSpecificCellMolecules(0, 0, 2) == cell000);    // replica
        CHECK(lattice.getSpecificCellMolecules(0, 0, 1) == cell001);    // original
        CHECK(lattice.getSpecificCellMolecules(0, 0, 3) == cell001);    // replica
    }
}