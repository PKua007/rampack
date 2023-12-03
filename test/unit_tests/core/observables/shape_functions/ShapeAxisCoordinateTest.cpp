//
// Created by Piotr Kubala on 24/03/2023.
//

#include "catch2/catch.hpp"

#include "mocks/MockShapeTraits.h"

#include "core/observables/shape_functions/ShapeAxisCoordinate.h"


TEST_CASE("ShapeAxisCoordinate") {
    MockShapeTraits traits;
    using trompeloeil::_;
    ALLOW_CALL(traits, getPrimaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    ALLOW_CALL(traits, getSecondaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{0, 1, 0});
    Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI/4));

    SECTION("primary axis") {
        ShapeAxisCoordinate shapeAxisCoordinate(ShapeGeometry::Axis::PRIMARY, 0);

        shapeAxisCoordinate.calculate(shape, traits);

        CHECK_THAT(shapeAxisCoordinate.getValues(), Catch::Matchers::Approx(std::vector<double>{M_SQRT1_2}));
    }

    SECTION("secondary axis") {
        ShapeAxisCoordinate shapeAxisCoordinate(ShapeGeometry::Axis::SECONDARY, 0);

        shapeAxisCoordinate.calculate(shape, traits);

        CHECK_THAT(shapeAxisCoordinate.getValues(), Catch::Matchers::Approx(std::vector<double>{-M_SQRT1_2}));
    }

    SECTION("auxiliary axis") {
        ShapeAxisCoordinate shapeAxisCoordinate(ShapeGeometry::Axis::AUXILIARY, 2);

        shapeAxisCoordinate.calculate(shape, traits);

        CHECK_THAT(shapeAxisCoordinate.getValues(), Catch::Matchers::Approx(std::vector<double>{1}));
    }
}