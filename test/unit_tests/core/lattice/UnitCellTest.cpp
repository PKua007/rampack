//
// Created by pkua on 21.05.22.
//

#include <catch2/catch.hpp>

#include "core/lattice/UnitCell.h"


TEST_CASE("UnitCell") {
    TriclinicBox box(std::array<double, 3>{1, 2, 4});
    Shape shape0({0.25, 0.5, 0.75});
    Shape shape1({0, 0.5, 0.5});
    UnitCell unitCell(box, {shape0, shape1});

    SECTION("read-only access") {
        CHECK(unitCell.size() == 2);
        CHECK(unitCell[0] == shape0);
        CHECK(unitCell[1] == shape1);
        CHECK_THROWS(unitCell[2]);
        CHECK(unitCell.getBox() == box);
        CHECK(unitCell.getMolecules() == std::vector<Shape>({shape0, shape1}));
        CHECK(unitCell.isNormalized());
        std::size_t i{};
        for (const auto &shape: unitCell) {
            if (i == 0)
                CHECK(shape == shape0);
            else if (i == 1)
                CHECK(shape == shape1);
            else
                FAIL("i > 1");
            i++;
        }
    }

    SECTION("operator[] modification") {
        unitCell[0] = shape1;

        CHECK(unitCell[0] == shape1);
    }

    SECTION("molecules vector modification") {
        unitCell.getMolecules()[0] = shape1;

        CHECK(unitCell[0] == shape1);
    }

    SECTION("iterator modification") {
        *unitCell.begin() = shape1;

        CHECK(unitCell[0] == shape1);
    }

    SECTION("box modification") {
        TriclinicBox newBox(std::array<double, 3>{1, 2, 8});

        unitCell.getBox() = newBox;

        CHECK(unitCell.getBox() == newBox);
    }
}

TEST_CASE("UnitCell: not normalized") {
    TriclinicBox box(std::array<double, 3>{1, 2, 4});

    SECTION("coord < 0") {
        UnitCell unitCell(box, {Shape({-0.25, 0.5, 0.75})});

        CHECK_FALSE(unitCell.isNormalized());
    }

    SECTION("coord >= 1") {
        UnitCell unitCell(box, {Shape({0.25, 0.5, 1.0})});

        CHECK_FALSE(unitCell.isNormalized());
    }
}

TEST_CASE("UnitCell: box modification via std::shared_ptr") {
    auto boxPtr = std::make_shared<TriclinicBox>(std::array<double, 3>{1, 2, 4});
    UnitCell unitCell(boxPtr, {Shape({0.5, 0.5, 0.5})});

    SECTION("modification") {
        *boxPtr = TriclinicBox(std::array<double, 3>{1, 2, 8});

        REQUIRE(unitCell.getBox() == TriclinicBox(std::array<double, 3>{1, 2, 8}));
    }

    SECTION("deep copy") {
        auto newCell = unitCell.deepCopy();
        *boxPtr = TriclinicBox(std::array<double, 3>{1, 2, 16});

        CHECK_FALSE(newCell.getBox() == TriclinicBox(std::array<double, 3>{1, 2, 8}));
    }
}