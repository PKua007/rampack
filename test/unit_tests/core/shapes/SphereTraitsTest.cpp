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
        CHECK(interaction.overlapBetweenShapes(sphere1, sphere2, pbc));
        CHECK(interaction.overlapBetweenShapes(sphere2, sphere1, pbc));
    }

    SECTION("non-overlapping") {
        Shape sphere1({1, 18.9, 10});
        Shape sphere2({5, 2, 10});
        CHECK_FALSE(interaction.overlapBetweenShapes(sphere1, sphere2, pbc));
        CHECK_FALSE(interaction.overlapBetweenShapes(sphere2, sphere1, pbc));
    }
}

TEST_CASE("Sphere: wall overlap") {
    SphereTraits sphereTraits(0.5);
    const Interaction &interaction = sphereTraits.getInteraction();

    CHECK(interaction.hasWallPart());

    SECTION("overlapping") {
        Shape sphere({0.4 + M_SQRT2/4, 0.4 + M_SQRT2/4, 5});
        CHECK(interaction.overlapWithWallForShape(sphere, {0, 1, 0}, {M_SQRT1_2, M_SQRT1_2, 0}));
    }

    SECTION("non-overlapping") {
        Shape sphere({0.6 + M_SQRT2/4, 0.6 + M_SQRT2/4, 5});
        CHECK_FALSE(interaction.overlapWithWallForShape(sphere, {0, 1, 0}, {M_SQRT1_2, M_SQRT1_2, 0}));
    }
}

TEST_CASE("Sphere: toWolfram") {
    SphereTraits sphereTraits(2);
    PeriodicBoundaryConditions pbc(10);
    Shape sphere({2, 4, 6});

    CHECK(sphereTraits.getPrinter("wolfram", {})->print(sphere) == "Sphere[{2, 4, 6},2]");
}

TEST_CASE("Sphere: geometry") {
    SphereTraits sphereTraits(2);

    CHECK_THROWS(sphereTraits.getGeometry().getPrimaryAxis(Shape{}));
    CHECK_THROWS(sphereTraits.getGeometry().getSecondaryAxis(Shape{}));
    CHECK(sphereTraits.getGeometry().getGeometricOrigin(Shape{}) == Vector<3>{0, 0, 0});
    CHECK(sphereTraits.getVolume() == Approx(32./3*M_PI));
}