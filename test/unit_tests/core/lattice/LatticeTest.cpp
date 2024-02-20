//
// Created by pkua on 21.05.22.
//

#include <catch2/catch.hpp>
#include <algorithm>
#include <iterator>

#include "matchers/VectorApproxMatcher.h"

#include "core/lattice/Lattice.h"


TEST_CASE("Lattice: operations") {
    TriclinicBox cellBox(std::array<double, 3>{1, 2, 3});
    Matrix<3, 3> rot1 = Matrix<3, 3>::identity();
    Matrix<3, 3> rot2 = Matrix<3, 3>::rotation(M_PI, 0, 0);
    ShapeData data1(double{1});
    ShapeData data2(double{2});
    std::vector<Shape> molecules{Shape({0, 0.25, 0.5}, rot1, data1), Shape({0.25, 0.5, 0.75}, rot2, data2)};
    UnitCell unitCell(cellBox, molecules);
    Lattice lattice(unitCell, {2, 3, 1});

    SECTION("read-only operations") {
        REQUIRE(lattice.isRegular());

        CHECK(lattice.getCellBox() == cellBox);
        CHECK(lattice.getUnitCell().getBox() == cellBox);
        CHECK(lattice.getUnitCell().getMolecules() == molecules);
        CHECK(lattice.getUnitCellMolecules() == molecules);
        CHECK(lattice.getSpecificCellMolecules(0, 1, 0) == molecules);
        CHECK(lattice.getSpecificCell(0, 1, 0).getBox() == cellBox);
        CHECK(lattice.getLatticeBox() == TriclinicBox(std::array<double, 3>{2, 6, 3}));
        CHECK(lattice.size() == 12);
        CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 3, 1});
        CHECK(lattice.isNormalized());

        // Should still be regular after those operations
        CHECK(lattice.isRegular());
    }

    SECTION("generating shapes") {
        auto shapes = lattice.generateMolecules();

        auto expectedShapes = std::vector<Shape>{
            Shape({0, 0.5, 1.5}, rot1, data1), Shape({0.25, 1.0, 2.25}, rot2, data2),
            Shape({1, 0.5, 1.5}, rot1, data1), Shape({1.25, 1.0, 2.25}, rot2, data2),
            Shape({0, 2.5, 1.5}, rot1, data1), Shape({0.25, 3.0, 2.25}, rot2, data2),
            Shape({1, 2.5, 1.5}, rot1, data1), Shape({1.25, 3.0, 2.25}, rot2, data2),
            Shape({0, 4.5, 1.5}, rot1, data1), Shape({0.25, 5.0, 2.25}, rot2, data2),
            Shape({1, 4.5, 1.5}, rot1, data1), Shape({1.25, 5.0, 2.25}, rot2, data2)
        };
        CHECK_THAT(shapes, Catch::Matchers::UnorderedEquals(expectedShapes));
    }

    SECTION("modifying unit cell dimensions") {
        TriclinicBox newBox(std::array<double, 3>{1, 2, 4});

        lattice.modifyCellBox() = newBox;

        CHECK(lattice.getCellBox() == newBox);
        CHECK(lattice.getSpecificCell(0, 1, 0).getBox() == newBox);
        CHECK(lattice.isRegular());
    }

    SECTION("modifying unit cell molecules") {
        lattice.modifyUnitCellMolecules()[0].setPosition({0.5, 0.5, 0.5});

        CHECK(lattice.getSpecificCell(0, 0, 0)[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});
        CHECK(lattice.getSpecificCell(0, 1, 0)[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});
        CHECK(lattice.getSpecificCell(0, 0, 0).size() == 2);
        CHECK(lattice.getSpecificCell(0, 1, 0).size() == 2);
        CHECK(lattice.isRegular());
        CHECK(lattice.size() == 12);

        SECTION("operations not throwing forrregular") {
            CHECK_NOTHROW(lattice.getUnitCell());
            CHECK_NOTHROW(lattice.getUnitCellMolecules());
            CHECK_NOTHROW(lattice.modifyUnitCellMolecules());
        }
    }

    SECTION("modifying a specific cell") {
        lattice.modifySpecificCellMolecules(0, 1, 0)[0].setPosition({0.5, 0.5, 0.5});
        auto &molecules2 = lattice.modifySpecificCellMolecules(0, 0, 0);
        molecules2.erase(molecules2.begin());

        CHECK(lattice.getSpecificCell(0, 1, 0)[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});   // Should be changed
        CHECK(lattice.getSpecificCell(0, 2, 0)[0].getPosition() == Vector<3>{0, 0.25, 0.5});    // Should not be changed
        CHECK(lattice.getSpecificCell(0, 0, 0).size() == 1);    // Should be changed
        CHECK(lattice.getSpecificCell(0, 1, 0).size() == 2);    // Should not be changed
        CHECK_FALSE(lattice.isRegular());
        CHECK(lattice.size() == 11);        // One particle less

        SECTION("operations throwing for irregular") {
            CHECK_THROWS(lattice.getUnitCell());
            CHECK_THROWS(lattice.getUnitCellMolecules());
            CHECK_THROWS(lattice.modifyUnitCellMolecules());
        }
    }
}

TEST_CASE("Lattice: not normalized") {
    SECTION("regular") {
        TriclinicBox cellBox(std::array<double, 3>{1, 2, 3});
        std::vector<Shape> molecules{Shape({0, -0.25, 0.5}), Shape({0.25, 0.5, 0.75})};
        UnitCell unitCell(cellBox, molecules);
        Lattice lattice(unitCell, {2, 3, 1});

        REQUIRE_FALSE(lattice.isNormalized());

        SECTION("normalization") {
            lattice.normalize();

            REQUIRE(lattice.isRegular());
            CHECK(lattice.getCellBox() == cellBox);
            std::vector<Shape> expectedShapes = std::vector<Shape>{Shape({0, 0.75, 0.5}), Shape({0.25, 0.5, 0.75})};
            CHECK_THAT(lattice.getUnitCellMolecules(), Catch::UnorderedEquals(expectedShapes));
            CHECK(lattice.size() == 12);
            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 3, 1});
            CHECK(lattice.isNormalized());
        }
    }

    SECTION("irregular") {
        // +-----+-----+
        // |  X  |     |
        // +-----+-----+
        // | X   |  XX |
        // +-----+-----+
        TriclinicBox cellBox(std::array<double, 3>{1, 2, 3});
        UnitCell unitCell(cellBox, {Shape({0.5, 0.5, 0.5})});
        Lattice lattice(unitCell, {2, 2, 1});
        lattice.modifySpecificCellMolecules(0, 0, 0)[0].setPosition({-0.25, 0.5, 0.5});
        lattice.modifySpecificCellMolecules(1, 1, 0)[0].setPosition({1.25, -0.5, 0.5});

        REQUIRE_FALSE(lattice.isNormalized());

        SECTION("normalization") {
            lattice.normalize();

            CHECK(lattice.getCellBox() == cellBox);
            const auto &molecules000 = lattice.getSpecificCellMolecules(0, 0, 0);
            const auto &molecules100 = lattice.getSpecificCellMolecules(1, 0, 0);
            const auto &molecules010 = lattice.getSpecificCellMolecules(0, 1, 0);
            const auto &molecules110 = lattice.getSpecificCellMolecules(1, 1, 0);
            REQUIRE(molecules000.size() == 1);
            CHECK_THAT(molecules000[0].getPosition(), IsApproxEqual(Vector<3>{0.25, 0.5, 0.5}, 1e-12));
            REQUIRE(molecules100.size() == 2);
            CHECK_THAT(molecules100[0].getPosition(), IsApproxEqual(Vector<3>{0.5, 0.5, 0.5}, 1e-12)
                                                      || IsApproxEqual(Vector<3>{0.75, 0.5, 0.5}, 1e-12));
            CHECK_THAT(molecules100[1].getPosition(), IsApproxEqual(Vector<3>{0.5, 0.5, 0.5}, 1e-12)
                                                      || IsApproxEqual(Vector<3>{0.75, 0.5, 0.5}, 1e-12));
            CHECK_THAT(molecules100[0].getPosition(), !IsApproxEqual(molecules100[1].getPosition(), 1e-12));
            REQUIRE(molecules010.size() == 1);
            CHECK_THAT(molecules010[0].getPosition(), IsApproxEqual(Vector<3>{0.5, 0.5, 0.5}, 1e-12));
            REQUIRE(molecules110.empty());
            CHECK(lattice.getDimensions() == std::array<std::size_t, 3>{2, 2, 1});
            CHECK(lattice.isNormalized());
        }
    }
}

TEST_CASE("Lattice: performing deep copy of a unit cell") {
    auto boxPtr = std::make_shared<TriclinicBox>(std::array<double, 3>{1, 2, 3});
    Lattice lattice(UnitCell(boxPtr, {Shape({0.5, 0.5, 0.5})}), {2, 3, 1});

    *boxPtr = TriclinicBox(std::array<double, 3>{1, 2, 6});

    CHECK(lattice.getCellBox() == TriclinicBox(std::array<double, 3>{1, 2, 3}));
}