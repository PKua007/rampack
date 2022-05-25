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

    const auto &data = traits.getSphereData();
    REQUIRE(data.size() == 3);
    CHECK(data[0].radius == 2);
    CHECK(data[1].radius == 3);
    CHECK(data[2].radius == 4);
    CHECK_THAT(data[0].position, IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK_THAT(data[1].position, IsApproxEqual({4, 0, 0}, 1e-12));
    CHECK_THAT(data[2].position, IsApproxEqual({10, 0, 0}, 1e-12));
    CHECK_THAT(traits.getGeometry().getGeometricOrigin(Shape{}), IsApproxEqual({6, 0, 0}, 1e-12));
}