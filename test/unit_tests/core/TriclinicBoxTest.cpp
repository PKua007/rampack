//
// Created by pkua on 02.02.2022.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/TriclinicBox.h"

TEST_CASE("TriclinicBox") {
    Vector<3> posRel1{0.5, 0.25, 0.5}, posRel2{0.75, 1.0, 0};
    Vector<3> posAbs1{9, 8, 5}, posAbs2{16, 22, 0};

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

    SECTION("sides") {
        auto sides = box.getSides();
        CHECK_THAT(sides[0], IsApproxEqual(Vector<3>{16, 8, 0}, 1e-12));
        CHECK_THAT(sides[1], IsApproxEqual(Vector<3>{4, 16, 0}, 1e-12));
        CHECK_THAT(sides[2], IsApproxEqual(Vector<3>{0, 0, 10}, 1e-12));
    }

    SECTION("heights") {
        auto heights = box.getHeights();
        CHECK(heights[0] == Approx(56 / std::sqrt(17)));
        CHECK(heights[1] == Approx(28 / std::sqrt(5)));
        CHECK(heights[2] == Approx(10));
    }

    SECTION("volume") {
        CHECK(box.getVolume() == Approx(2240));
    }

    SECTION("transform") {
        box.transform(Matrix<3, 3>{1, 0, 0,
                                   0, 2, 0,
                                   0, 0, 3});

        CHECK(box.getDimensions() == Matrix<3, 3>{16,  4,  0,
                                                  16, 32,  0,
                                                   0,  0, 30});

        SECTION("self-consistency") {
            CHECK_THAT(box.relativeToAbsolute(box.absoluteToRelative(Vector<3>{1, 2, 3})),
                       IsApproxEqual(Vector<3>{1, 2, 3}, 1e-12));
        }
    }
}
