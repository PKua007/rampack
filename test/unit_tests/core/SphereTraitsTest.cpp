//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "core/shapes/SphereTraits.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("Sphere: construction") {
    SECTION("default radius") {
        SphereTraits sphereTraits;

        CHECK(sphereTraits.getRadius() == 1);
    }

    SECTION("custom") {
        SphereTraits sphereTraits(3);

        CHECK(sphereTraits.getRadius() == 3);
    }
}

TEST_CASE("Sphere: overlap") {
    // Shapes positions create Pythagorean triangle 3, 4, 5 (through y-axis pbc) on z = 10 plane and are exactly touching
    // in scale 1
    SphereTraits sphereTraits(2.5);
    PeriodicBoundaryConditions pbc(20);
    Shape sphere1({1, 19, 10});
    Shape sphere2({5, 2, 10});

    SECTION("overlapping scale") {
        CHECK(sphereTraits.overlapBetween(sphere1, sphere2, 0.999, pbc));
        CHECK(sphereTraits.overlapBetween(sphere2, sphere1, 0.999, pbc));
    }

    SECTION("non-overlapping scale") {
        CHECK_FALSE(sphereTraits.overlapBetween(sphere1, sphere2, 1.001, pbc));
        CHECK_FALSE(sphereTraits.overlapBetween(sphere2, sphere1, 1.001, pbc));
    }
}

TEST_CASE("Sphere: toWolfram") {
    SphereTraits sphereTraits(2);
    PeriodicBoundaryConditions pbc(10);
    Shape sphere({1, 2, 3});

    CHECK(sphereTraits.getPrinter().toWolfram(sphere, 2) == "Sphere[{2, 4, 6},2]");
}