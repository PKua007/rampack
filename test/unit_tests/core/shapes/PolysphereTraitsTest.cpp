//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "core/shapes/PolysphereTraits.h"
#include "core/PeriodicBoundaryConditions.h"

#include "matchers/VectorApproxMatcher.h"

namespace {
    class DummyInteraction : public CentralInteraction {
    protected:
        [[nodiscard]] double calculateEnergyForDistance2([[maybe_unused]] double distance) const override { return 0; }
    };
}

TEST_CASE("PolysphereTraits: hard interactions") {
    PolysphereTraits::PolysphereGeometry geometry({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}},
                                                  {1, 0, 0}, {0, 1, 0}, {0.5, 0, 0});
    PolysphereTraits traits(std::move(geometry));

    SECTION("hard interactions") {
        // Particles look and are placed like this (x - central particle, o - second one):
        //
        //       1   4   6   9
        //   ||  x-->o   o<--x  ||
        //
        // 4, 6 should be tangent on scale 10. Each particle should be reflected separately! If all are reflected
        // simultaneously, than they are no longer tangent

        const Interaction &interaction = traits.getInteraction();
        PeriodicBoundaryConditions pbc(10);

        CHECK(interaction.hasHardPart());
        CHECK_FALSE(interaction.hasSoftPart());

        SECTION("overlap") {
            Shape shape1({1.01, 5, 5}, Matrix<3, 3>::identity());
            Shape shape2({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0));
            CHECK(interaction.overlapBetweenShapes(shape1, shape2, pbc));
        }

        SECTION("no overlap") {
            Shape shape1({0.99, 5, 5}, Matrix<3, 3>::identity());
            Shape shape2({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0));
            CHECK_FALSE(interaction.overlapBetweenShapes(shape1, shape2, pbc));
        }
    }

    SECTION("overlap with wall") {
        const Interaction &interaction = traits.getInteraction();

        CHECK(interaction.hasWallPart());

        SECTION("overlapping") {
            Shape shape({1.1, 1.1, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI/2));
            CHECK(interaction.overlapWithWallForShape(shape, {0, 5, 0}, {0, -1, 0}));
        }

        SECTION("non-overlapping") {
            Shape shape({0.9, 0.9, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI/2));
            CHECK_FALSE(interaction.overlapWithWallForShape(shape, {0, 5, 0}, {0, -1, 0}));
        }
    }

    SECTION("toWolfram") {
        Shape shape({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0));

        std::string expected = "{Sphere[{9.000000, 5.000000, 5.000000},0.500000],"
                               "Sphere[{6.000000, 5.000000, 5.000000},1.000000]}";
        CHECK(traits.getPrinter("wolfram").toWolfram(shape) == expected);
    }

    SECTION("getVolume") {
        CHECK(traits.getGeometry().getVolume() == Approx(4.71238898038469));
    }

    SECTION("primary axis") {
        // primary axis X rotated 90 deg around z axis => primary axis is Y
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2));
        CHECK_THAT(traits.getGeometry().getPrimaryAxis(shape), IsApproxEqual({0, 1, 0}, 1e-8));
    }

    SECTION("secondary axis") {
        // secondary axis Y rotated 90 deg around z axis => secondary axis is -X
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2));
        CHECK_THAT(traits.getGeometry().getSecondaryAxis(shape), IsApproxEqual({-1, 0, 0}, 1e-8));
    }

    SECTION("geometric origin") {
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2));
        CHECK_THAT(traits.getGeometry().getGeometricOrigin(shape), IsApproxEqual({0, 0.5, 0}, 1e-8));
    }
}

TEST_CASE("PolysphereTraits: soft interactions") {
    PolysphereTraits::PolysphereGeometry geometry({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}}, {1, 0, 0}, {0, 1, 0}, {0, 0, 0});
    PolysphereTraits traits(std::move(geometry), std::make_unique<DummyInteraction>());
    const auto &interaction = dynamic_cast<const CentralInteraction &>(traits.getInteraction());

    CHECK(interaction.getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}, {3, 0, 0}});
}

TEST_CASE("PolysphereTraits: mass centre normalization") {
    PolysphereTraits::PolysphereGeometry geometry({{{0, 0, 0}, 1}, {{1, 0, 0}, std::cbrt(3)}},
                                                  {1, 0, 0}, {0, 1, 0}, {1, 0, 0},
                                                  {{"point1", {1, 0, 0}}});
    geometry.normalizeMassCentre();
    PolysphereTraits traits(geometry);

    const auto &sphereData = traits.getSphereData();
    CHECK(sphereData == std::vector<PolysphereTraits::SphereData>{{{-0.75, 0, 0}, 1}, {{0.25, 0, 0}, std::cbrt(3)}});
    CHECK_THAT(geometry.getGeometricOrigin({}), IsApproxEqual(Vector<3>{0.25, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("point1", {}), IsApproxEqual(Vector<3>{0.25, 0, 0}, 1e-12));
}

TEST_CASE("PolysphereTraits: named points") {
    PolysphereTraits::PolysphereGeometry geometry({{{0, 0, 0}, 1}, {{1, 0, 0}, 1}},
                                                  {1, 0, 0}, {0, 1, 0}, {1, 0, 0},
                                                  {{"named1", {0, 2, 0}}});

    Shape shape({1, 2, 3}, Matrix<3, 3>::rotation(0, 0, M_PI/2));
    CHECK_THAT(geometry.getNamedPointForShape("s0", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("s1", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 1, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("named1", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{-2, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("o", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 1, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("cm", shape), IsApproxEqual({1, 2, 3}, 1e-12));
}