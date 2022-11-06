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
    double largeSpherePenetration = 2;
    PolysphereLollipopTraits traits(sphereNum, smallSphereRadius, largeSphereRadius, smallSpherePenetration,
                                    largeSpherePenetration, false);

    SECTION("sphere data") {
        const auto &data = traits.getSphereData();
        REQUIRE(data.size() == 3);
        CHECK(data[0].radius == 2);
        CHECK(data[1].radius == 2);
        CHECK(data[2].radius == 4);
        CHECK_THAT(data[0].position, IsApproxEqual({0, 0, 0}, 1e-12));
        CHECK_THAT(data[1].position, IsApproxEqual({3, 0, 0}, 1e-12));
        CHECK_THAT(data[2].position, IsApproxEqual({7, 0, 0}, 1e-12));
    }

    SECTION("geometry") {
        const auto &geometry = traits.getGeometry();
        Shape shape({1, 2, 3}, Matrix<3, 3>::rotation(0, 0, M_PI/2));

        CHECK_THAT(geometry.getGeometricOrigin(shape), IsApproxEqual({0, 4.5, 0}, 1e-12));
        CHECK_THAT(geometry.getPrimaryAxis(shape), IsApproxEqual({0, 1, 0}, 1e-12));
        CHECK_THAT(geometry.getSecondaryAxis(shape), IsApproxEqual({-1, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("s0", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("ss", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("s2", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 7, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("sl", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 7, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("o", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 4.5, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("cm", shape), IsApproxEqual({1, 2, 3}, 1e-12));
    }
}