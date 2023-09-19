//
// Created by Piotr Kubala on 19/09/2023.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"

#include "core/observables/shape_functions/ShapeQTensor.h"


TEST_CASE("ShapeQTensor") {
    MockShapeTraits traits;
    using trompeloeil::_;
    ALLOW_CALL(traits, getPrimaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    ALLOW_CALL(traits, getSecondaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{0, 1, 0});
    Shape shape({}, Matrix<3, 3>::rotation(Vector<3>{1, 1, 1}.normalized(), M_PI/3));

    using vd = std::vector<double>;

    SECTION("primary axis") {
        ShapeQTensor shapeQTensor(ShapeGeometry::Axis::PRIMARY);

        shapeQTensor.calculate(shape, traits);

        CHECK_THAT(shapeQTensor.getValues(), Catch::Matchers::Approx(vd{1./6, 2./3, -1./3, 1./6, -1./3, -1./3}));
        CHECK(shapeQTensor.getPrimaryName() == "Q_pa");
    }

    SECTION("secondary axis") {
        ShapeQTensor shapeQTensor(ShapeGeometry::Axis::SECONDARY);

        shapeQTensor.calculate(shape, traits);

        CHECK_THAT(shapeQTensor.getValues(), Catch::Matchers::Approx(vd{-1./3, -1./3, -1./3, 1./6, 2./3, 1./6}));
        CHECK(shapeQTensor.getPrimaryName() == "Q_sa");
    }

    SECTION("auxiliary axis") {
        ShapeQTensor shapeQTensor(ShapeGeometry::Axis::AUXILIARY);

        shapeQTensor.calculate(shape, traits);

        CHECK_THAT(shapeQTensor.getValues(), Catch::Matchers::Approx(vd{1./6, -1./3, 2./3, -1./3, -1./3, 1./6}));
        CHECK(shapeQTensor.getPrimaryName() == "Q_aa");
    }

    SECTION("names") {
        std::vector<std::string> expected = {"xx", "xy", "xz", "yy", "yz", "zz"};
        CHECK(ShapeQTensor(ShapeGeometry::Axis::PRIMARY).getNames() == expected);
    }
}