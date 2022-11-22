//
// Created by pkua on 22.11.22.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "geometry/Quaternion.h"


TEST_CASE("Quaterion") {
    SECTION("identity") {
        auto q = Quaternion::fromMatrix(Matrix<3, 3>::identity());
        CHECK(q == Vector<4>{0, 0, 0, 1});
    }

    SECTION("x rotation") {
        auto q = Quaternion::fromMatrix(Matrix<3, 3>::rotation(M_PI/2, 0, 0));
        CHECK_THAT(q, IsApproxEqual(Vector<4>{M_SQRT1_2, 0, 0, M_SQRT1_2}, 1e-14));
    }

    SECTION("arbitrary") {
        Vector<3> axis{1, 2, 3};
        axis = axis.normalized();
        double angle = 1.2;
        Vector<4> expected;
        std::copy(axis.begin(), axis.end(), expected.begin());
        expected *= std::sin(angle/2);
        expected[3] = std::cos(angle/2);

        auto q = Quaternion::fromMatrix(Matrix<3, 3>::rotation(axis, angle));
        CHECK_THAT(q, IsApproxEqual(expected, 1e-14));
    }
}