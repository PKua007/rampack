//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "core/LatticeArrangingModel.h"
#include "core/FreeBoundaryConditions.h"

namespace {
    class DummyShape : public Shape {
    public:
        [[nodiscard]] bool overlap([[maybe_unused]] const Shape &other, [[maybe_unused]] double scaleFactor,
                                   [[maybe_unused]] const BoundaryConditions &bc) const override
        {
            return false;
        }

        [[nodiscard]] std::unique_ptr<Shape> clone() const override {
            return std::make_unique<DummyShape>(*this);
        }
    };
}

TEST_CASE("LatticeArrangingModel: not fully filled lattice") {
    LatticeArrangingModel model;
    DummyShape dummyShape;
    FreeBoundaryConditions fbc;

    auto shapes = model.arrange(dummyShape, 7, 2, fbc);

    REQUIRE(shapes.size() == 7);
    CHECK(shapes[0]->getPosition() == std::array<double, 3>{0.5, 0.5, 0.5});
    CHECK(shapes[1]->getPosition() == std::array<double, 3>{0.5, 0.5, 1.5});
    CHECK(shapes[2]->getPosition() == std::array<double, 3>{0.5, 1.5, 0.5});
    CHECK(shapes[3]->getPosition() == std::array<double, 3>{0.5, 1.5, 1.5});
    CHECK(shapes[4]->getPosition() == std::array<double, 3>{1.5, 0.5, 0.5});
    CHECK(shapes[5]->getPosition() == std::array<double, 3>{1.5, 0.5, 1.5});
    CHECK(shapes[6]->getPosition() == std::array<double, 3>{1.5, 1.5, 0.5});
}

TEST_CASE("LatticeArrangingModel: fully filled lattice") {
    LatticeArrangingModel model;
    DummyShape dummyShape;
    FreeBoundaryConditions fbc;

    auto shapes = model.arrange(dummyShape, 8, 2, fbc);

    REQUIRE(shapes.size() == 8);
    CHECK(shapes[0]->getPosition() == std::array<double, 3>{0.5, 0.5, 0.5});
    CHECK(shapes[1]->getPosition() == std::array<double, 3>{0.5, 0.5, 1.5});
    CHECK(shapes[2]->getPosition() == std::array<double, 3>{0.5, 1.5, 0.5});
    CHECK(shapes[3]->getPosition() == std::array<double, 3>{0.5, 1.5, 1.5});
    CHECK(shapes[4]->getPosition() == std::array<double, 3>{1.5, 0.5, 0.5});
    CHECK(shapes[5]->getPosition() == std::array<double, 3>{1.5, 0.5, 1.5});
    CHECK(shapes[6]->getPosition() == std::array<double, 3>{1.5, 1.5, 0.5});
    CHECK(shapes[6]->getPosition() == std::array<double, 3>{1.5, 1.5, 1.5});
}