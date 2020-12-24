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
    const Interaction &interaction = sphereTraits.getInteraction();
    PeriodicBoundaryConditions pbc(20);

    SECTION("overlapping") {
        Shape sphere1({1, 19.1, 10});
        Shape sphere2({5, 2, 10});
        CHECK(interaction.overlapBetween(sphere1, sphere2, pbc));
        CHECK(interaction.overlapBetween(sphere2, sphere1, pbc));
    }

    SECTION("non-overlapping") {
        Shape sphere1({1, 18.9, 10});
        Shape sphere2({5, 2, 10});
        CHECK_FALSE(interaction.overlapBetween(sphere1, sphere2, pbc));
        CHECK_FALSE(interaction.overlapBetween(sphere2, sphere1, pbc));
    }
}

TEST_CASE("Sphere: toWolfram") {
    SphereTraits sphereTraits(2);
    PeriodicBoundaryConditions pbc(10);
    Shape sphere({2, 4, 6});

    CHECK(sphereTraits.getPrinter().toWolfram(sphere) == "Sphere[{2, 4, 6},2]");
}