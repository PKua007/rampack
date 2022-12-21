//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "frontend/legacy/ShapeFactory.h"
#include "core/shapes/SphereTraits.h"

TEST_CASE("ShapeFactory: Sphere") {
    SECTION("valid") {
        auto traits = legacy::ShapeFactory::shapeTraitsFor("Sphere", "2", "");
        auto sphereTraits = dynamic_cast<SphereTraits*>(traits.get());
        REQUIRE(sphereTraits != nullptr);
        CHECK(sphereTraits->getRadius() == Approx(2));
    }

    SECTION("invalid") {
        CHECK_THROWS(legacy::ShapeFactory::shapeTraitsFor("Sphere", "", ""));
        CHECK_THROWS(legacy::ShapeFactory::shapeTraitsFor("Sphere", "0", ""));
        CHECK_THROWS(legacy::ShapeFactory::shapeTraitsFor("Sphere", "a", ""));
    }
}

TEST_CASE("ShapeFactory: unknown") {
    CHECK_THROWS(legacy::ShapeFactory::shapeTraitsFor("NonExisting", "Foo", ""));
}