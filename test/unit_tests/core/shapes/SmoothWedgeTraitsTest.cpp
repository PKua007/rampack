//
// Created by pkua on 02.10.22.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/SmoothWedgeTraits.h"


TEST_CASE("SmoothWedge: geometry") {
    SmoothWedgeTraits traits(2, 1, 5);
    const auto &geometry = traits.getGeometry();
    const auto &interaction = traits.getInteraction();

    Shape shape({1, 2, 3}, Matrix<3, 3>::rotation(0, M_PI/2, 0));
    CHECK_THAT(geometry.getPrimaryAxis(shape), IsApproxEqual({1, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getSecondaryAxis(shape), IsApproxEqual({0, 0, -1}, 1e-12));
    CHECK_THAT(geometry.getGeometricOrigin(shape), IsApproxEqual({-0.5, 0, 0}, 1e-12));
    // Calculated in Mathematica using the formula, cross-checked with numerical convex hull volume
    CHECK(geometry.getVolume() == Approx(272*M_PI/15));
    CHECK_THAT(geometry.getNamedPoint("cm", shape), IsApproxEqual({1, 2, 3}, 1e-12));
    CHECK_THAT(geometry.getNamedPoint("o", shape), IsApproxEqual({0.5, 2, 3}, 1e-12));
    CHECK_THAT(geometry.getNamedPoint("ss", shape), IsApproxEqual({3.5, 2, 3}, 1e-12));
    CHECK_THAT(geometry.getNamedPoint("sl", shape), IsApproxEqual({-1.5, 2, 3}, 1e-12));
    CHECK(interaction.getRangeRadius() == 9);
}

TEST_CASE("SmoothWedge: collide geometry") {
    SmoothWedgeTraits traits(2, 1, 5);
    const auto &collideGeometry = traits.getCollideGeometry();

    CHECK_THAT(collideGeometry.getCenter(), IsApproxEqual({0, 0, 0}, 1e-12));

    // We are always using not normalized normal vectors, because they should work
    CHECK_THAT(collideGeometry.getSupportPoint({0, 0, 2}), IsApproxEqual({0, 0, 3.5}, 1e-12));
    CHECK_THAT(collideGeometry.getSupportPoint({0, 0, -2}), IsApproxEqual({0, 0, -4.5}, 1e-12));
    double b = 0.2;
    double h1 = 0.4*std::sqrt(6);
    auto sideNormal = Vector<3>{h1, 0, b};
    auto tangentPoint = Vector<3>{h1, 0, 2.5 + b};
    CHECK(sideNormal * (collideGeometry.getSupportPoint(sideNormal) - tangentPoint) == Approx(0).margin(1e-12));
}