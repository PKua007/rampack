//
// Created by pkua on 15.09.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"

#include "core/observables/correlation/S110Correlation.h"


TEST_CASE("S110Correlation") {
    MockShapeTraits traits;
    using trompeloeil::_;
    ALLOW_CALL(traits, getPrimaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0} + _1.getPosition());
    ALLOW_CALL(traits, getSecondaryAxis(ANY(Shape))).RETURN(_1.getOrientation() * Vector<3>{0, 0, 1} + _1.getPosition());
    Shape s1, s2({}, Matrix<3, 3>::rotation(0, 0, M_PI/4));

    SECTION("primary axis") {
        S110Correlation s110Correlation(S110Correlation::Axis::PRIMARY_AXIS);
        CHECK(s110Correlation.calculate(s1, s2, traits) == Approx(M_SQRT1_2));
    }

    SECTION("secondary axis") {
        S110Correlation s110Correlation(S110Correlation::Axis::SECONDARY_AXIS);
        CHECK(s110Correlation.calculate(s1, s2, traits) == Approx(1));
    }
}