//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "core/LatticeArrangingModel.h"

TEST_CASE("LatticeArrangingModel: not fully filled lattice") {
    LatticeArrangingModel model;

    auto shapes = model.arrange(7, 2);

    REQUIRE(shapes.size() == 7);
    CHECK(shapes[0]->getPosition() == Vector<3>{0.5, 0.5, 0.5});
    CHECK(shapes[1]->getPosition() == Vector<3>{0.5, 0.5, 1.5});
    CHECK(shapes[2]->getPosition() == Vector<3>{0.5, 1.5, 0.5});
    CHECK(shapes[3]->getPosition() == Vector<3>{0.5, 1.5, 1.5});
    CHECK(shapes[4]->getPosition() == Vector<3>{1.5, 0.5, 0.5});
    CHECK(shapes[5]->getPosition() == Vector<3>{1.5, 0.5, 1.5});
    CHECK(shapes[6]->getPosition() == Vector<3>{1.5, 1.5, 0.5});
}

TEST_CASE("LatticeArrangingModel: fully filled lattice") {
    LatticeArrangingModel model;

    auto shapes = model.arrange(8, 2);

    REQUIRE(shapes.size() == 8);
    CHECK(shapes[0]->getPosition() == Vector<3>{0.5, 0.5, 0.5});
    CHECK(shapes[1]->getPosition() == Vector<3>{0.5, 0.5, 1.5});
    CHECK(shapes[2]->getPosition() == Vector<3>{0.5, 1.5, 0.5});
    CHECK(shapes[3]->getPosition() == Vector<3>{0.5, 1.5, 1.5});
    CHECK(shapes[4]->getPosition() == Vector<3>{1.5, 0.5, 0.5});
    CHECK(shapes[5]->getPosition() == Vector<3>{1.5, 0.5, 1.5});
    CHECK(shapes[6]->getPosition() == Vector<3>{1.5, 1.5, 0.5});
    CHECK(shapes[7]->getPosition() == Vector<3>{1.5, 1.5, 1.5});
}