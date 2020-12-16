//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "frontend/ShapeFactory.h"
#include "core/shapes/Sphere.h"

TEST_CASE("ShapeFactory: Sphere") {
    SECTION("valid") {
        auto shape = ShapeFactory::createShape("Sphere", "2");
        auto sphere = dynamic_cast<Sphere*>(shape.get());
        REQUIRE(sphere != nullptr);
        CHECK(sphere->getRadius() == Approx(2));
        CHECK(sphere->getPosition() == Vector<3>{0, 0, 0});
    }

    SECTION("invalid") {
        CHECK_THROWS(ShapeFactory::createShape("Sphere", ""));
        CHECK_THROWS(ShapeFactory::createShape("Sphere", "0"));
        CHECK_THROWS(ShapeFactory::createShape("Sphere", "a"));
    }
}

TEST_CASE("ShapeFactory: unknown") {
    CHECK_THROWS(ShapeFactory::createShape("NonExisting", "Foo"));
}