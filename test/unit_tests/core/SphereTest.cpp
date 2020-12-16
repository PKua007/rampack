//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "core/shapes/Sphere.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("Sphere: construction") {
    SECTION("default radius") {
        Sphere sphere;

        CHECK(sphere.getRadius() == 1);
    }

    SECTION("custom") {
        Sphere sphere(3);

        CHECK(sphere.getRadius() == 3);
    }
}

TEST_CASE("Sphere: overlap") {
    // Shapes positions create Pythagorean triangle 3, 4, 5 (through y-axis pbc) on z = 10 plane and are exactly touching
    // in scale 1
    Sphere sphere1(2), sphere2(3);
    PeriodicBoundaryConditions pbc(20);
    sphere1.translate({1, 19, 10}, pbc);
    sphere2.translate({5, 2, 10}, pbc);

    SECTION("overlapping scale") {
        CHECK(sphere1.overlap(sphere2, 0.999, pbc));
        CHECK(sphere2.overlap(sphere1, 0.999, pbc));
    }

    SECTION("non-overlapping scale") {
        CHECK_FALSE(sphere1.overlap(sphere2, 1.001, pbc));
        CHECK_FALSE(sphere2.overlap(sphere1, 1.001, pbc));
    }
}

TEST_CASE("Sphere: toWolfram") {
    Sphere sphere(2);
    PeriodicBoundaryConditions pbc(10);
    sphere.translate({1, 2, 3}, pbc);

    CHECK(sphere.toWolfram(2) == "Sphere[{2, 4, 6},2]");
}