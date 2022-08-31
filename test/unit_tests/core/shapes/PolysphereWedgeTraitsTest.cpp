//
// Created by pkua on 09.03.2022.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolysphereWedgeTraits.h"


TEST_CASE("PolysphereWedgeTraits") {
    std::size_t sphereNum = 3;
    double smallSphereRadius = 2;
    double largeSphereRadius = 4;
    double spherePenetration = 1;
    PolysphereWedgeTraits traits(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration, false);

    SECTION("sphere data") {
        const auto &data = traits.getSphereData();
        REQUIRE(data.size() == 3);
        CHECK(data[0].radius == 2);
        CHECK(data[1].radius == 3);
        CHECK(data[2].radius == 4);
        CHECK_THAT(data[0].position, IsApproxEqual({0, 0, 0}, 1e-12));
        CHECK_THAT(data[1].position, IsApproxEqual({4, 0, 0}, 1e-12));
        CHECK_THAT(data[2].position, IsApproxEqual({10, 0, 0}, 1e-12));
    }

    SECTION("geometry") {
        const auto &geometry = traits.getGeometry();
        Shape shape({1, 2, 3}, Matrix<3, 3>::rotation(0, 0, M_PI/2));

        CHECK_THAT(geometry.getGeometricOrigin(shape), IsApproxEqual({0, 6, 0}, 1e-12));
        CHECK_THAT(geometry.getPrimaryAxis(shape), IsApproxEqual({0, 1, 0}, 1e-12));
        CHECK_THAT(geometry.getSecondaryAxis(shape), IsApproxEqual({-1, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("s0", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("ss", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("s2", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 10, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("sl", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 10, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("o", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 6, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("cm", shape), IsApproxEqual({1, 2, 3}, 1e-12));
    }
}