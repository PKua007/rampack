//
// Created by Piotr Kubala on 01/09/2023.
//

#include "catch2/catch.hpp"

#include "mocks/MockShapeTraits.h"

#include "core/observables/correlation_functions/S221Correlation.h"


TEST_CASE("S221Correlation") {
    MockShapeTraits traits;
    using trompeloeil::_;
    ALLOW_CALL(traits, getPrimaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    Shape s1, s2({}, Matrix<3, 3>::rotation(0, 0, M_PI/3));
    S221Correlation s221Correlation(ShapeGeometry::Axis::PRIMARY);

    CHECK(s221Correlation.calculate(s1, s2, {0, 0, -1}, traits) == Approx(-std::sqrt(3)/4));
}