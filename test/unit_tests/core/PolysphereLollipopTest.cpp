//
// Created by pkua on 01.03.2022.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolysphereLollipopTraits.h"


TEST_CASE("PolysphereLollipopTraits") {
    std::size_t sphereNum = 3;
    double smallSphereRadius = 2;
    double largeSphereRadius = 4;
    double smallSpherePenetration = 1;
    double largeSpherePenertation = 2;
    PolysphereLollipopTraits traits(sphereNum, smallSphereRadius, largeSphereRadius, smallSpherePenetration,
                                    largeSpherePenertation, false);

    const auto &data = traits.getSphereData();
    REQUIRE(data.size() == 3);
    CHECK(data[0].radius == 2);
    CHECK(data[1].radius == 2);
    CHECK(data[2].radius == 4);
    CHECK_THAT(data[0].position, IsApproxEqual({0, 0, 0}, 1e-12));
    CHECK_THAT(data[1].position, IsApproxEqual({3, 0, 0}, 1e-12));
    CHECK_THAT(data[2].position, IsApproxEqual({7, 0, 0}, 1e-12));
}