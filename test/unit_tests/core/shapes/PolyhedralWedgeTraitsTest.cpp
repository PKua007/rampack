//
// Created by Piotr Kubala on 19/04/2023.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolyhedralWedgeTraits.h"
#include "core/FreeBoundaryConditions.h"


TEST_CASE("PolyhedralWedgeTraits: geometry") {
    PolyhedralWedgeTraits traits(3, 1, 2, 4, 6);
    const auto &geometry = traits.getGeometry();
    const auto &interaction = traits.getInteraction();

    Shape shape({1, 2, 3}, Matrix<3, 3>::rotation(0, M_PI/2, 0));
    CHECK_THAT(geometry.getPrimaryAxis(shape), IsApproxEqual({1, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getSecondaryAxis(shape), IsApproxEqual({0, 0, -1}, 1e-12));
    CHECK_THAT(geometry.getGeometricOrigin(shape), IsApproxEqual({0, 0, 0}, 1e-12));
    // Calculated in Mathematica using Volume@ConvexHull@vertices
    CHECK(geometry.getVolume({}) == Approx(36));
    CHECK(geometry.getNamedPoint("beg") == Vector<3>{0, 0, -3});
    CHECK(geometry.getNamedPoint("end") == Vector<3>{0, 0, 3});
    CHECK(interaction.getRangeRadius() == Approx(2*std::sqrt(14)));
}

TEST_CASE("PolyhedralWedgeTraits: overlap") {
    auto subdivisions = GENERATE(0, 3);

    DYNAMIC_SECTION("subdivisions: " << subdivisions) {
        PolyhedralWedgeTraits traits(3, 1, 2, 4, 6, subdivisions);
        const auto &interaction = traits.getInteraction();
        FreeBoundaryConditions fbc;
        Shape shape0;

        // This case was visually inspected using Mathematica
        SECTION("vertex-face") {
            auto rot1 = Matrix<3, 3>::rotation(0, 1, 0.5);
            Shape shapeOv1({-2.92, -2.92, 2.92}, rot1);
            Shape shapeNonOv1({-2.93, -2.93, 2.93}, rot1);

            CHECK(interaction.overlapBetweenShapes(shape0, shapeOv1, fbc));
            CHECK_FALSE(interaction.overlapBetweenShapes(shape0, shapeNonOv1, fbc));
        }

        // Face-face (this one is easily visualized in your head, c'mon)
        SECTION("face-face") {
            auto rot2 = Matrix<3, 3>::rotation(0, 0, 0.5);
            Shape shapeOv2({0, 0, 5.99}, rot2);
            Shape shapeNonOv2({0, 0, 6.01}, rot2);

            CHECK(interaction.overlapBetweenShapes(shape0, shapeOv2, fbc));
            CHECK_FALSE(interaction.overlapBetweenShapes(shape0, shapeNonOv2, fbc));
        }
    }
}