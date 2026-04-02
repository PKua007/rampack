//
// Created by pkua on 09.03.2022.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolysphereWedgeTraits.h"


TEST_CASE("PolysphereWedgeTraits") {
    std::size_t sphereNum = 3;
    double bottomSphereRadius = 4;
    double topSphereRadius = 2;
    double spherePenetration = 1;
    PolysphereWedgeTraits traits(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration);

    SECTION("sphere data") {
        const auto &data = traits.getSphereData();
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

        CHECK_THAT(geometry.getGeometricOrigin({}), IsApproxEqual({0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getPrimaryAxis({}), IsApproxEqual({0, 0, 1}, 1e-12));
        CHECK(geometry.getVolume() == Approx(6205*M_PI/48));     // Mathematica value
        CHECK_THAT(geometry.getNamedPoint("s0"), IsApproxEqual({0, 0, -4}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("beg"), IsApproxEqual({0, 0, -4}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("s2"), IsApproxEqual({0, 0, 6}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("end"), IsApproxEqual({0, 0, 6}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("o"), IsApproxEqual({0, 0, 0}, 1e-12));
        // Named point "cm" has its own test
    }
}

TEST_CASE("PolysphereWedgeTraits: mass centre") {
    SECTION("existing") {
        PolysphereWedgeTraits traits(3, 1, 2, 0);

        CHECK_THAT(traits.getGeometry().getNamedPoint("cm"), IsApproxEqual({0, 0, 35./33}, 1e-12));
    }

    SECTION("not existing") {
        PolysphereWedgeTraits traits(3, 1, 2, 0.1);

        CHECK_FALSE(traits.getGeometry().hasNamedPoint("cm"));
    }
}

TEST_CASE("PolysphereWedgeTraits: interaction centre layout") {
    SECTION("different radii") {
        PolysphereWedgeTraits traits(3, 4, 2, 1);
        const auto &geometry = dynamic_cast<const PolysphereTraits::PolysphereGeometry &>(traits.getGeometry());

        CHECK(geometry.getInteractionCentreLayout().getCentreIdxTypeIdxMap() == std::vector<std::size_t>{0, 1, 2});
        CHECK(geometry.getDisplayRadiiByType() == std::vector<double>{4, 3, 2});
    }

    SECTION("equal radii remain distinguishable") {
        PolysphereWedgeTraits traits(3, 2, 2, 0);
        const auto &geometry = dynamic_cast<const PolysphereTraits::PolysphereGeometry &>(traits.getGeometry());

        CHECK(geometry.getInteractionCentreLayout().getCentreIdxTypeIdxMap() == std::vector<std::size_t>{0, 1, 2});
        CHECK(geometry.getDisplayRadiiByType() == std::vector<double>{2, 2, 2});
    }
}
