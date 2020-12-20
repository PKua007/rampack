//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "core/Shape.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("Shape: construction") {
    SECTION("default") {
        Shape shape;

        REQUIRE(shape.getPosition() == Vector<3>{0, 0, 0});
        REQUIRE(shape.getOrientation() == Matrix<3, 3>::identity());
    }

    SECTION("specific position") {
        Shape shape({1, 2, 3});

        REQUIRE(shape.getPosition() == Vector<3>{1, 2, 3});
        REQUIRE(shape.getOrientation() == Matrix<3, 3>::identity());
    }

    SECTION("specific position and orientation") {
        Shape shape({1, 2, 3},
                         {-1,  0, 0,
                           0, -1, 0,
                           0,  0, 1});

        REQUIRE(shape.getPosition() == Vector<3>{1, 2, 3});
        REQUIRE(shape.getOrientation() == Matrix<3, 3>{-1,  0, 0,
                                                        0, -1, 0,
                                                        0,  0, 1});
    }
}

TEST_CASE("Shape: translation with pbc") {
    Shape shape({1, 2, 3});
    PeriodicBoundaryConditions pbc(4);

    shape.translate({2, -1, 7}, pbc);

    REQUIRE(shape.getPosition() == Vector<3>{3, 1, 2});
}

TEST_CASE("Shape: orientation") {
    Shape shape{};

    shape.rotate({1,  0,  0,
                  0, -1,  0,
                  0,  0, -1});

    REQUIRE(shape.getOrientation() == Matrix<3, 3>{1,  0,  0,
                                                   0, -1,  0,
                                                   0,  0, -1});
}