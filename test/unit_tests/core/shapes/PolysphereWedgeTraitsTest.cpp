//
// Created by pkua on 09.03.2022.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolysphereWedgeTraits.h"


namespace {
    const Shape defaultShape({}, Matrix<3, 3>::identity(), PolysphereTraits::Data{0});
}

TEST_CASE("PolysphereWedgeTraits") {
    std::size_t sphereNum = 3;
    double bottomSphereRadius = 4;
    double topSphereRadius = 2;
    double spherePenetration = 1;
    PolysphereWedgeTraits traits(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration);

    SECTION("sphere data") {
        const auto &data = traits.getDefaultSpecies().getSphereData();
        REQUIRE(data.size() == 3);
        CHECK(data[0].radius == 4);
        CHECK(data[1].radius == 3);
        CHECK(data[2].radius == 2);
        CHECK_THAT(data[0].position, IsApproxEqual({0, 0, -4}, 1e-12));
        CHECK_THAT(data[1].position, IsApproxEqual({0, 0, 2}, 1e-12));
        CHECK_THAT(data[2].position, IsApproxEqual({0, 0, 6}, 1e-12));
    }

    SECTION("geometry") {
        const auto &geometry = traits.getGeometry();

        CHECK_THAT(geometry.getGeometricOrigin(defaultShape), IsApproxEqual({0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getPrimaryAxis(defaultShape), IsApproxEqual({0, 0, 1}, 1e-12));
        CHECK(geometry.getVolume(defaultShape) == Approx(6205*M_PI/48));     // Mathematica value
        CHECK_THAT(geometry.getNamedPoint("s0").forShape(defaultShape), IsApproxEqual({0, 0, -4}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("beg").forShape(defaultShape), IsApproxEqual({0, 0, -4}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("s2").forShape(defaultShape), IsApproxEqual({0, 0, 6}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("end").forShape(defaultShape), IsApproxEqual({0, 0, 6}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("o").forShape(defaultShape), IsApproxEqual({0, 0, 0}, 1e-12));
        // Named point "cm" has its own test
    }
}

TEST_CASE("PolysphereWedgeTraits: mass centre") {
    SECTION("existing") {
        PolysphereWedgeTraits traits(3, 1, 2, 0);

        CHECK_THAT(traits.getGeometry().getNamedPoint("cm").forShape(defaultShape),
                   IsApproxEqual({0, 0, 35./33}, 1e-12));
    }

    SECTION("not existing") {
        PolysphereWedgeTraits traits(3, 1, 2, 0.1);

        CHECK_FALSE(traits.getGeometry().hasNamedPoint("cm"));
    }
}