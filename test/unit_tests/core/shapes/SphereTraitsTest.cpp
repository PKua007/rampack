//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "mocks/MockCentralInteraction.h"

#include "core/shapes/SphereTraits.h"
#include "core/PeriodicBoundaryConditions.h"


namespace {
    using SphereData = SphereTraits::HardData;
}

TEST_CASE("SphereTraits: overlap") {
    // Shapes positions create Pythagorean triangle 3, 4, 5 (through y-axis pbc) on z = 10 plane and are exactly
    // touching in scale 1
    SphereTraits sphereTraits;
    const Interaction &interaction = sphereTraits.getInteraction();
    PeriodicBoundaryConditions pbc(20);
    auto noRot = Matrix<3, 3>::identity();

    CHECK(interaction.hasHardPart());

    SECTION("overlapping") {
        Shape sphere1({1, 19.1, 10}, noRot, SphereData{2});
        Shape sphere2({5, 2, 10}, noRot, SphereData{3});
        CHECK(interaction.overlapBetweenShapes(sphere1, sphere2, pbc));
        CHECK(interaction.overlapBetweenShapes(sphere2, sphere1, pbc));
    }

    SECTION("non-overlapping") {
        Shape sphere1({1, 18.9, 10}, noRot, SphereData{2});
        Shape sphere2({5, 2, 10}, noRot, SphereData{3});
        CHECK_FALSE(interaction.overlapBetweenShapes(sphere1, sphere2, pbc));
        CHECK_FALSE(interaction.overlapBetweenShapes(sphere2, sphere1, pbc));
    }
}

TEST_CASE("SphereTraits: wall overlap") {
    SphereTraits sphereTraits;
    const Interaction &interaction = sphereTraits.getInteraction();
    auto noRot = Matrix<3, 3>::identity();

    CHECK(interaction.hasWallPart());

    SECTION("overlapping") {
        Shape sphere({0.4 + M_SQRT2/4, 0.4 + M_SQRT2/4, 5}, noRot, SphereData{0.5});
        CHECK(interaction.overlapWithWallForShape(sphere, {0, 1, 0}, {M_SQRT1_2, M_SQRT1_2, 0}));
    }

    SECTION("non-overlapping") {
        Shape sphere({0.6 + M_SQRT2/4, 0.6 + M_SQRT2/4, 5}, noRot, SphereData{0.5});
        CHECK_FALSE(interaction.overlapWithWallForShape(sphere, {0, 1, 0}, {M_SQRT1_2, M_SQRT1_2, 0}));
    }
}

TEST_CASE("SphereTraits: toWolfram") {
    PeriodicBoundaryConditions pbc(10);

    SECTION("hard interaction") {
        SphereTraits sphereTraits;
        Shape sphere({2, 4, 6}, Matrix<3, 3>::identity(), SphereTraits::HardData{2});

        CHECK(sphereTraits.getPrinter("wolfram", {})->print(sphere)
              == "Sphere[{2, 4, 6},2]");
    }

    SECTION("soft interaction") {
        SphereTraits sphereTraits(2, std::make_shared<MockCentralInteraction>());
        Shape sphere({2, 4, 6});

        CHECK(sphereTraits.getPrinter("wolfram", {})->print(sphere)
              == "Sphere[{2, 4, 6},2]");
    }
}

TEST_CASE("SphereTraits: geometry") {
    SECTION("hard interaction") {
        SphereTraits sphereTraits;
        const auto &geometry = sphereTraits.getGeometry();
        Shape sphere({2, 4, 6}, Matrix<3, 3>::identity(), SphereTraits::HardData{2});

        CHECK_THROWS(geometry.getPrimaryAxis(sphere));
        CHECK_THROWS(geometry.getSecondaryAxis(sphere));
        CHECK(geometry.getGeometricOrigin(sphere) == Vector<3>{0, 0, 0});
        CHECK(geometry.getVolume(sphere) == Approx(32./3*M_PI));
    }

    SECTION("soft interaction") {
        SphereTraits sphereTraits(2, std::make_shared<MockCentralInteraction>());
        Shape sphere({2, 4, 6});

        CHECK(sphereTraits.getGeometry().getVolume(sphere) == Approx(32./3*M_PI));
    }
}

TEST_CASE("SphereTraits: serialization") {
    SECTION("hard interaction") {
        SECTION("default data") {
            SphereTraits traits(2);

            SphereTraits::HardData expected{2};
            CHECK(traits.getDataManager().defaultDeserialize({}) == ShapeData(expected));
        }

        SECTION("serialization & deserialization") {
            SphereTraits traits;
            const auto &manager = traits.getDataManager();

            TextualShapeData textualData{{"r", "2.5"}};
            ShapeData shapeData(SphereData{2.5});
            CHECK(manager.serialize(shapeData) == textualData);
            CHECK(manager.deserialize(textualData) == shapeData);
        }
    }

    SECTION("soft interaction") {
        SphereTraits sphereTraits(2, std::make_shared<MockCentralInteraction>());
        const auto &manager = sphereTraits.getDataManager();

        CHECK(manager.serialize({}).empty());
        CHECK(manager.deserialize({}).isEmpty());
    }
}