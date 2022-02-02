//
// Created by pkua on 02.02.2022.
//

#include <catch2/catch.hpp>

#include "core/boxes/OrthorhombicBox.h"

TEST_CASE("OrthorhombicBox") {
    Vector<3> posRel1{0, 0.25, 0.5}, posRel2{0.25, 0.5, 0.75};
    Vector<3> posAbs1{0, 5, 20}, posAbs2{2.5, 10, 30};
    Shape shapeRel1(posRel1), shapeRel2(posRel2);
    Shape shapeAbs1(posAbs1), shapeAbs2(posAbs2);
    std::vector<Shape> relativeShapes{shapeRel1, shapeRel2};
    std::vector<Shape> absoluteShapes{shapeAbs1, shapeAbs2};

    OrthorhombicBox box({10, 20, 40});

    SECTION("Vector<3>") {
        SECTION("relative to absolute") {
            CHECK(posAbs1 == box.relativeToAbsolute(posRel1));
        }

        SECTION("absolute to relative") {
            CHECK(posRel1 == box.absoluteToRelative(posAbs1));
        }
    }

    SECTION("shapes") {
        SECTION("relative to absolute") {
            box.absoluteToRelative(absoluteShapes);
            CHECK(absoluteShapes == relativeShapes);
        }

        SECTION("absolute to relative") {
            box.relativeToAbsolute(relativeShapes);
            CHECK(relativeShapes == absoluteShapes);
        }
    }
}