//
// Created by pkua on 02.02.2022.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/boxes/TriclinicBox.h"

TEST_CASE("TriclinicBox") {
    Vector<3> posRel1{0.5, 0.25, 0.5}, posRel2{0.75, 1.0, 0};
    Vector<3> posAbs1{9, 8, 5}, posAbs2{16, 22, 0};
    Shape shapeRel1(posRel1), shapeRel2(posRel2);
    Shape shapeAbs1(posAbs1), shapeAbs2(posAbs2);
    std::vector<Shape> relativeShapes{shapeRel1, shapeRel2};
    std::vector<Shape> absoluteShapes{shapeAbs1, shapeAbs2};

    // Test both ways of creating the box
    auto box = GENERATE(
        TriclinicBox(std::array<Vector<3>, 3>{{{16, 8, 0}, {4, 16, 0}, {0, 0, 10}}}),
        TriclinicBox(Matrix<3, 3>{16,  4,  0,
                                   8, 16,  0,
                                   0,  0, 10})
    );

    SECTION("Vector<3>") {
        SECTION("relative to absolute") {
            CHECK_THAT(box.relativeToAbsolute(posRel1), IsApproxEqual(posAbs1, 1e-12));
        }

        SECTION("absolute to relative") {
            CHECK_THAT(box.absoluteToRelative(posAbs1), IsApproxEqual(posRel1, 1e-12));
        }
    }

    SECTION("shapes") {
        SECTION("relative to absolute") {
            box.absoluteToRelative(absoluteShapes);
            for (std::size_t i{}; i < 2; i++)
                CHECK_THAT(absoluteShapes[i].getPosition(), IsApproxEqual(relativeShapes[i].getPosition(), 1e-12));
        }

        SECTION("absolute to relative") {
            box.relativeToAbsolute(relativeShapes);
            for (std::size_t i{}; i < 2; i++)
                CHECK_THAT(relativeShapes[i].getPosition(), IsApproxEqual(absoluteShapes[i].getPosition(), 1e-12));
        }
    }
}