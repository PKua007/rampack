//
// Created by pkua on 02.10.22.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/SmoothWedgeTraits.h"


TEST_CASE("SmoothWedge: geometry") {
    SmoothWedgeTraits traits;
    ShapeData data = traits.shapeDataFor(2, 1, 5);
    const auto &geometry = traits.getGeometry();
    const auto &interaction = traits.getInteraction();

    Shape shape({1, 2, 3}, Matrix<3, 3>::rotation(0, M_PI/2, 0), data);
    CHECK_THAT(geometry.getPrimaryAxis(shape), IsApproxEqual({1, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getGeometricOrigin(shape), IsApproxEqual({0, 0, 0}, 1e-12));
    // Calculated in Mathematica using the analytic formula, cross-checked with numerical convex hull volume
    CHECK(geometry.getVolume(shape) == Approx(272*M_PI/15));
    CHECK_THAT(geometry.getNamedPointForShape("o", shape), IsApproxEqual({1, 2, 3}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("beg", shape), IsApproxEqual({-1.0, 2, 3}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("end", shape), IsApproxEqual({4.0, 2, 3}, 1e-12));
    CHECK(interaction.getRangeRadius(data.raw()) == 8);
}

TEST_CASE("SmoothWedge: collide geometry") {
    double b = 0.2;     // distance from sphere center to sphere-side tangent point (along the height)
    double h1 = 0.4 * std::sqrt(6);     // orthogonal distance from sphere-side tangent point to the height
    auto sideNormal = Vector<3>{h1, 0, b};      // normal at tangent point
    auto tangentPoint = Vector<3>{h1, 0, 3 + b};

    SECTION("spheres") {
        SECTION("original case") {
            SmoothWedgeTraits traits;
            ShapeData data = traits.shapeDataFor(2, 1, 5);
            const auto &collideGeometry = traits.getCollideGeometry(data.raw());

            CHECK(collideGeometry.getInsphereRadius() == Approx(1.6));
            CHECK(collideGeometry.getCircumsphereRadius() == Approx(4));
        }

        SECTION("case detecting bug") {
            SmoothWedgeTraits traits;
            ShapeData data = traits.shapeDataFor(3, 1, 5);
            const auto &collideGeometry = traits.getCollideGeometry(data.raw());

            CHECK(collideGeometry.getInsphereRadius() == Approx(2.4));
            CHECK(collideGeometry.getCircumsphereRadius() == Approx(4.5));
        }
    }

    SECTION("no subdivisions") {
        SmoothWedgeTraits traits;
        ShapeData data = traits.shapeDataFor(2, 1, 5);
        const auto &collideGeometry = traits.getCollideGeometry(data.raw());

        CHECK_THAT(collideGeometry.getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));

        // We are always using not normalized normal vectors, because they should work
        CHECK_THAT(collideGeometry.getSupportPoint({0, 0, 2}), IsApproxEqual({0, 0, 4}, 1e-12));    // sphere 1
        CHECK_THAT(collideGeometry.getSupportPoint({0, 0, -2}), IsApproxEqual({0, 0, -4}, 1e-12));  // sphere 2
        // side
        CHECK(sideNormal * (collideGeometry.getSupportPoint(sideNormal) - tangentPoint) == Approx(0).margin(1e-12));
    }

    SECTION("3 subdivisions") {
        SmoothWedgeTraits traits;
        ShapeData data = traits.shapeDataFor(2, 1, 5, 3);
        const auto &collideGeometry0 = traits.getCollideGeometry(data.raw(), 0);
        const auto &collideGeometry1 = traits.getCollideGeometry(data.raw(), 1);
        const auto &collideGeometry2 = traits.getCollideGeometry(data.raw(), 2);
        const auto &centers = traits.getInteractionCentres(data.raw());

        CHECK_THAT(collideGeometry0.getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
        CHECK_THAT(collideGeometry1.getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
        CHECK_THAT(collideGeometry2.getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));
        REQUIRE(centers.size() == 3);

        // We are always using not normalized normal vectors, because they should work
        CHECK_THAT(collideGeometry2.getSupportPoint({0, 0, 2}) + centers[2], IsApproxEqual({0, 0, 4}, 1e-12));
        CHECK_THAT(collideGeometry0.getSupportPoint({0, 0, -2}) + centers[0], IsApproxEqual({0, 0, -4}, 1e-12));
        auto approx0 = Approx(0).margin(1e-12);
        // side for all 3 subdivisions
        CHECK(sideNormal * (collideGeometry0.getSupportPoint(sideNormal) - tangentPoint + centers[0]) == approx0);
        CHECK(sideNormal * (collideGeometry1.getSupportPoint(sideNormal) - tangentPoint + centers[1]) == approx0);
        CHECK(sideNormal * (collideGeometry2.getSupportPoint(sideNormal) - tangentPoint + centers[2]) == approx0);
    }
}

TEST_CASE("SmoothWedgeTraits: serialization") {
    SECTION("default data") {
        SmoothWedgeTraits traits(1, 2, 3, 4);

        ShapeData expected = traits.shapeDataFor(1, 2, 3, 4);
        CHECK(traits.getDataManager().defaultDeserialize({}) == expected);
    }

    SECTION("serialization & deserialization") {
        SmoothWedgeTraits traits;
        const auto &manager = traits.getDataManager();

        TextualShapeData textualData{{"bottom_r", "1"}, {"top_r", "2"}, {"l", "3"}, {"subdivisions", "4"}};
        ShapeData shapeData = traits.shapeDataFor(1, 2, 3, 4);
        CHECK(manager.serialize(shapeData) == textualData);
        CHECK(manager.deserialize(textualData) == shapeData);
    }
}

TEST_CASE("SmoothWedgeTraits: shape data consistency") {
    SmoothWedgeTraits traits(1, 2, 3, 4);

    ShapeData data = traits.shapeDataFor(1, 2, 3, 4);
    ShapeData differentData = traits.shapeDataFor(5, 6, 7, 8);
    ShapeData defaultDeserializedData = traits.defaultDeserialize({});
    ShapeData deserializedData = traits.deserialize(
        {{"bottom_r", "1"}, {"top_r", "2"}, {"l", "3"}, {"subdivisions", "4"}}
    );

    CHECK_FALSE(data == differentData);
    CHECK(data == defaultDeserializedData);
    CHECK(data == deserializedData);
}
