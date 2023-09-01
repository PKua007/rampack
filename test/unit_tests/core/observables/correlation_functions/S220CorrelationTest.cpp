//
// Created by Piotr Kubala on 01/09/2023.
//

#include "catch2/catch.hpp"

#include "mocks/MockShapeTraits.h"

#include "core/observables/correlation_functions/S220Correlation.h"


TEST_CASE("S220Correlation") {
    MockShapeTraits traits;
    using trompeloeil::_;
    ALLOW_CALL(traits, getPrimaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    Shape s1, s2({}, Matrix<3, 3>::rotation(0, 0, M_PI/4));
    S220Correlation s220Correlation(ShapeGeometry::Axis::PRIMARY);

    CHECK(s220Correlation.calculate(s1, s2, {1, 0, 0}, traits) == Approx(0.25));
    CHECK(s220Correlation.getSignatureName() == "S220");
}