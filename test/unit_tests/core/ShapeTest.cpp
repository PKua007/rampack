//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "core/Shape.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    class DummyShape : public Shape {
    public:
        DummyShape() = default;
        explicit DummyShape(const std::array<double, 3> &position) : Shape(position) { }

        [[nodiscard]] bool overlap([[maybe_unused]] const Shape &other, [[maybe_unused]] double scaleFactor,
                                   [[maybe_unused]] const BoundaryConditions &bc) const override
        {
            return false;
        }

        [[nodiscard]] std::unique_ptr<Shape> clone() const override {
            return std::make_unique<DummyShape>(*this);
        }

        [[nodiscard]] std::string toWolfram([[maybe_unused]] double scaleFactor) const override { return ""; }
        [[nodiscard]] double getVolume() const override { return 1; }
    };
}

TEST_CASE("Shape: construction") {
    SECTION("default") {
        DummyShape shape;

        REQUIRE(shape.getPosition() == Vector<3>{0, 0, 0});
    }

    SECTION("specific") {
        DummyShape shape({1, 2, 3});

        REQUIRE(shape.getPosition() == Vector<3>{1, 2, 3});
    }
}

TEST_CASE("Shape: translation with pbc") {
    DummyShape shape({1, 2, 3});
    PeriodicBoundaryConditions pbc(4);

    shape.translate({2, -1, 7}, pbc);

    REQUIRE(shape.getPosition() == Vector<3>{3, 1, 2});
}