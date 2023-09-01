//
// Created by Piotr Kubala on 01/09/2023.
//

#include "catch2/catch.hpp"

#include "mocks/MockShapeTraits.h"

#include "core/observables/correlation_functions/AxesAngle.h"


TEST_CASE("AxesAngle") {
    MockShapeTraits traits;
    using trompeloeil::_;
    ALLOW_CALL(traits, getPrimaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    Shape s1, s2({}, Matrix<3, 3>::rotation(0, 0, M_PI/3));
    AxesAngle axesAngle(ShapeGeometry::Axis::PRIMARY);

    CHECK(axesAngle.calculate(s1, s2, {1, 0, 0}, traits) == Approx(60));
    CHECK(axesAngle.getSignatureName() == "theta");
}