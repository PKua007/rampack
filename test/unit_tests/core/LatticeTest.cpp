//
// Created by pkua on 21.05.22.
//

#include <catch2/catch.hpp>
#include <algorithm>
#include <iterator>

#include "core/lattice/Lattice.h"

TEST_CASE("Lattice") {
    TriclinicBox cellBox(std::array<double, 3>{1, 2, 3});
    UnitCell unitCell(cellBox, {Shape({0, 0.25, 0.5}), Shape({0.25, 0.5, 0.75})});
    Lattice lattice(unitCell, {2, 3, 1});

    SECTION("read-only operations") {
        REQUIRE(lattice.isRegular());

        CHECK(lattice.getCellBox() == cellBox);
        CHECK(lattice.getCell(0, 0, 0).getBox() == cellBox);
        CHECK(lattice.getLatticeBox() == TriclinicBox(std::array<double, 3>{2, 6, 3}));
        CHECK(lattice.size() == 12);
        CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 3, 1});

        // Should still be regular after those operations
        CHECK(lattice.isRegular());
    }

    SECTION("generating shapes") {
        auto shapes = lattice.generateMolecules();
        std::vector<Vector<3>> pos;
        std::transform(shapes.begin(), shapes.end(), std::back_inserter(pos), [](const auto &shape) {
            return shape.getPosition();
        });

        auto shouldContain = std::vector<Vector<3>>{
                {0, 0.5, 1.5}, {0.25, 1.0, 2.25}, {1, 0.5, 1.5}, {1.25, 1.0, 2.25},
                {0, 2.5, 1.5}, {0.25, 3.0, 2.25}, {1, 2.5, 1.5}, {1.25, 3.0, 2.25},
                {0, 4.5, 1.5}, {0.25, 5.0, 2.25}, {1, 4.5, 1.5}, {1.25, 5.0, 2.25},
        };
        CHECK_THAT(pos, Catch::Matchers::UnorderedEquals(shouldContain));
    }

    SECTION("modifying unit cell dimensions") {
        TriclinicBox newBox(std::array<double, 3>{1, 2, 4});

        lattice.getCellBox() = newBox;

        CHECK(lattice.getCellBox() == newBox);
        CHECK(lattice.getCell(0, 1, 0).getBox() == newBox);
        CHECK(lattice.isRegular());
    }

    SECTION("modifying a cell") {
        lattice.modifyCellMolecules(0, 1, 0)[0].setPosition({0.5, 0.5, 0.5});
        auto &molecules2 = lattice.modifyCellMolecules(0, 0, 0);
        molecules2.erase(molecules2.begin());

        CHECK(lattice.getCell(0, 1, 0)[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});   // Should be changed
        CHECK(lattice.getCell(0, 2, 0)[0].getPosition() == Vector<3>{0, 0.25, 0.5});    // Should not be changed
        CHECK(lattice.getCell(0, 0, 0).size() == 1);    // Should be changed
        CHECK(lattice.getCell(0, 1, 0).size() == 2);    // Should not be changed
        CHECK_FALSE(lattice.isRegular());
        CHECK(lattice.size() == 11);        // One particle less
    }
}