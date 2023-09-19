//
// Created by Piotr Kubala on 19/09/2023.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"

#include "core/observables/shape_functions/ShapeAxis.h"


TEST_CASE("ShapeAxis") {
    MockShapeTraits traits;
    using trompeloeil::_;
    ALLOW_CALL(traits, getPrimaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    ALLOW_CALL(traits, getSecondaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{0, 1, 0});
    Shape shape({}, Matrix<3, 3>::rotation(Vector<3>{1, 1, 1}.normalized(), M_PI/3));

    using vd = std::vector<double>;

    SECTION("primary axis") {
        ShapeAxis shapeAxis(ShapeGeometry::Axis::PRIMARY);

        shapeAxis.calculate(shape, traits);

        CHECK_THAT(shapeAxis.getValues(), Catch::Matchers::Approx(vd{2./3, 2./3, -1./3}));
        CHECK(shapeAxis.getPrimaryName() == "pa");
    }

    SECTION("secondary axis") {
        ShapeAxis shapeAxis(ShapeGeometry::Axis::SECONDARY);

        shapeAxis.calculate(shape, traits);

        CHECK_THAT(shapeAxis.getValues(), Catch::Matchers::Approx(vd{-1./3, 2./3, 2./3}));
        CHECK(shapeAxis.getPrimaryName() == "sa");
    }

    SECTION("auxiliary axis") {
        ShapeAxis shapeAxis(ShapeGeometry::Axis::AUXILIARY);

        shapeAxis.calculate(shape, traits);

        CHECK_THAT(shapeAxis.getValues(), Catch::Matchers::Approx(vd{2./3, -1./3, 2./3}));
        CHECK(shapeAxis.getPrimaryName() == "aa");
    }

    SECTION("names") {
        std::vector<std::string> expected = {"x", "y", "z"};
        CHECK(ShapeAxis(ShapeGeometry::Axis::PRIMARY).getNames() == expected);
    }
}