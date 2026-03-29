//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "core/interactions/CentralInteraction.h"
#include "core/shapes/PolysphereTraits.h"
#include "core/PeriodicBoundaryConditions.h"

#include "matchers/VectorApproxMatcher.h"

namespace {
    using PolysphereGeometry = PolysphereTraits::PolysphereGeometry;
    using SphereData = PolysphereTraits::SphereData;
    using Metadata = PolysphereTraits::InteractionCentreTypeMetadata;
    using LayoutWithMetadata = PolysphereTraits::InteractionCentreLayoutWithMetadata;

    class PairDataInteraction : public CentralInteraction<PairDataInteraction, double> {
    public:
        using CentralInteraction<PairDataInteraction, double>::CentralInteraction;

        [[nodiscard]] double calculateEnergyForDistance2([[maybe_unused]] double distance2,
                                                         const double &pairData) const
        {
            return pairData;
        }

        [[nodiscard]] static double getRangeRadiusForPairData(const double &pairData)
        {
            return pairData;
        }
    };
}

TEST_CASE("PolysphereGeometry: construction") {
    SECTION("legacy construction") {
        PolysphereGeometry geometry({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}}, {1, 0, 0}, {0, 1, 0}, {0.5, 0, 0});

        CHECK(geometry.getSphereData() == std::vector<SphereData>{{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}});
        CHECK(geometry.getInteractionCentreLayout().getCentres() == std::vector<Vector<3>>{{0, 0, 0}, {3, 0, 0}});
        CHECK(geometry.getInteractionCentreLayout().getCentreIdxTypeMap() == std::vector<std::size_t>{0, 1});
        CHECK(geometry.getDisplayRadiiByType() == std::vector<double>{0.5, 1});

        Shape shape({}, Matrix<3, 3>::identity());
        CHECK_THAT(geometry.getNamedPointForShape("s0", shape), IsApproxEqual(Vector<3>{0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("s1", shape), IsApproxEqual(Vector<3>{3, 0, 0}, 1e-12));
    }

    SECTION("typed construction") {
        LayoutWithMetadata layoutWithMetadata({{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}}, {1, 0, 0, 1}},
                                              {Metadata(0.25), Metadata(0.5)});
        PolysphereGeometry geometry(layoutWithMetadata, {1, 0, 0}, {0, 1, 0}, {0, 0, 0});

        CHECK(geometry.getSphereData() == std::vector<SphereData>{
                                            {{0, 0, 0}, 0.5}, {{1, 0, 0}, 0.25}, {{2, 0, 0}, 0.25}, {{3, 0, 0}, 0.5}});
        CHECK(geometry.getInteractionCentreLayout().getCentres()
              == std::vector<Vector<3>>{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}});
        CHECK(geometry.getInteractionCentreLayout().getCentreIdxTypeMap() == std::vector<std::size_t>{1, 0, 0, 1});
        CHECK(geometry.getDisplayRadiiByType() == std::vector<double>{0.25, 0.5});

        Shape shape({}, Matrix<3, 3>::identity());
        CHECK_THAT(geometry.getNamedPointForShape("s0", shape), IsApproxEqual(Vector<3>{0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("s1", shape), IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("s2", shape), IsApproxEqual(Vector<3>{2, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPointForShape("s3", shape), IsApproxEqual(Vector<3>{3, 0, 0}, 1e-12));
    }

    SECTION("invalid typed construction") {
        CHECK_THROWS_AS(LayoutWithMetadata({{{0, 0, 0}, {1, 0, 0}}, {0, 1}}, {Metadata(1.0)}), PreconditionException);
        CHECK_THROWS_AS(LayoutWithMetadata({{{0, 0, 0}, {1, 0, 0}}, {0, 1}},
                                           {Metadata(1.0), Metadata(2.0), Metadata(3.0)}), PreconditionException);
    }
}

TEST_CASE("PolysphereGeometry: basic properties") {
    PolysphereGeometry geometry({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}}, {1, 0, 0}, {0, 1, 0}, {0.5, 0, 0});

    SECTION("getVolume") {
        CHECK(geometry.getVolume() == Approx(4.71238898038469));
    }

    SECTION("primary axis") {
        // primary axis X rotated 90 deg around z axis => primary axis is Y
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2));
        CHECK_THAT(geometry.getPrimaryAxis(shape), IsApproxEqual({0, 1, 0}, 1e-8));
    }

    SECTION("secondary axis") {
        // secondary axis Y rotated 90 deg around z axis => secondary axis is -X
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2));
        CHECK_THAT(geometry.getSecondaryAxis(shape), IsApproxEqual({-1, 0, 0}, 1e-8));
    }

    SECTION("geometric origin") {
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2));
        CHECK_THAT(geometry.getGeometricOrigin(shape), IsApproxEqual({0, 0.5, 0}, 1e-8));
    }
}

TEST_CASE("PolysphereTraits: soft interaction binding") {
    PeriodicBoundaryConditions pbc(100);
    const Matrix<3, 3> id = Matrix<3, 3>::identity();

    SECTION("broadcast enabled for legacy geometry") {
        PolysphereGeometry geometry({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}}, {1, 0, 0}, {0, 1, 0}, {0, 0, 0});
        auto interaction = std::make_shared<PairDataInteraction>(2.5);
        PolysphereTraits traits(std::move(geometry), interaction, true);
        const auto &boundInteraction = traits.getInteraction();

        CHECK(boundInteraction.getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}, {3, 0, 0}});
        CHECK(boundInteraction.calculateEnergyBetween({0, 0, 0}, id, 0, {0, 0, 0}, id, 0, pbc) == Approx(2.5));
        CHECK(boundInteraction.calculateEnergyBetween({0, 0, 0}, id, 0, {0, 0, 0}, id, 1, pbc) == Approx(2.5));
        CHECK(boundInteraction.calculateEnergyBetween({0, 0, 0}, id, 1, {0, 0, 0}, id, 1, pbc) == Approx(2.5));
    }

    SECTION("broadcast disabled for legacy geometry") {
        PolysphereGeometry geometry({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}}, {1, 0, 0}, {0, 1, 0}, {0, 0, 0});
        auto interaction = std::make_shared<PairDataInteraction>(2.5);

        CHECK_THROWS_AS(PolysphereTraits(std::move(geometry), interaction), PreconditionException);
    }

    SECTION("typed geometry binds without broadcast when pair data map is sufficient") {
        LayoutWithMetadata layoutWithMetadata({{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}}, {1, 0, 1}},
                                              {Metadata(0.5), Metadata(1.0)});
        PolysphereGeometry geometry(layoutWithMetadata, {1, 0, 0}, {0, 1, 0}, {0, 0, 0}, 1.0);
        CentrePairDataMap<double> pairDataMap(2);
        pairDataMap.setPairData(0, 0, 1.0);
        pairDataMap.setPairData(0, 1, 2.0);
        pairDataMap.setPairData(1, 1, 3.0);
        auto interaction = std::make_shared<PairDataInteraction>(pairDataMap);
        PolysphereTraits traits(std::move(geometry), interaction);
        const auto &boundInteraction = traits.getInteraction();

        CHECK(boundInteraction.getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}});
        CHECK(boundInteraction.calculateEnergyBetween({0, 0, 0}, id, 1, {0, 0, 0}, id, 1, pbc) == Approx(1.0));
        CHECK(boundInteraction.calculateEnergyBetween({0, 0, 0}, id, 0, {0, 0, 0}, id, 1, pbc) == Approx(2.0));
        CHECK(boundInteraction.calculateEnergyBetween({0, 0, 0}, id, 0, {0, 0, 0}, id, 2, pbc) == Approx(3.0));
    }
}

TEST_CASE("PolysphereGeometry: mass centre normalization") {
    double volume = 1;     // Volume is not important here, we are lazy and choose an arbitrary number
    LayoutWithMetadata layoutWithMetadata({{{0, 0, 0}, {1, 0, 0}}, {1, 0}},
                                          {Metadata(std::cbrt(3)), Metadata(1.0)});
    PolysphereGeometry geometry(layoutWithMetadata, {1, 0, 0}, {0, 1, 0}, {1, 0, 0}, volume, {{"point1", {1, 0, 0}}});
    geometry.normalizeMassCentre();

    const auto &sphereData = geometry.getSphereData();
    CHECK(sphereData == std::vector<SphereData>{{{-0.75, 0, 0}, 1}, {{0.25, 0, 0}, std::cbrt(3)}});
    CHECK(geometry.getInteractionCentreLayout().getCentres() == std::vector<Vector<3>>{{-0.75, 0, 0}, {0.25, 0, 0}});
    CHECK(geometry.getInteractionCentreLayout().getCentreIdxTypeMap() == std::vector<std::size_t>{1, 0});
    CHECK(geometry.getDisplayRadiiByType() == std::vector<double>{std::cbrt(3), 1});
    CHECK_THAT(geometry.getGeometricOrigin({}), IsApproxEqual(Vector<3>{0.25, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("point1", {}), IsApproxEqual(Vector<3>{0.25, 0, 0}, 1e-12));
}

TEST_CASE("PolysphereGeometry: named points") {
    double volume = 1;     // Volume is not important here, we are lazy and choose an arbitrary number
    PolysphereGeometry geometry({{{0, 0, 0}, 1}, {{1, 0, 0}, 1}}, {1, 0, 0}, {0, 1, 0}, {1, 0, 0}, volume,
                                {{"named1", {0, 2, 0}}});

    Shape shape({1, 2, 3}, Matrix<3, 3>::rotation(0, 0, M_PI/2));
    CHECK_THAT(geometry.getNamedPointForShape("s0", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("s1", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 1, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("named1", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{-2, 0, 0}, 1e-12));
    CHECK_THAT(geometry.getNamedPointForShape("o", shape), IsApproxEqual(Vector<3>{1, 2, 3} + Vector<3>{0, 1, 0}, 1e-12));
}

TEST_CASE("PolysphereTraits: hard interaction") {
    PolysphereGeometry geometry({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}}, {1, 0, 0}, {0, 1, 0}, {0.5, 0, 0});
    PolysphereTraits traits(std::move(geometry));
    const Interaction &interaction = traits.getInteraction();

    SECTION("hard interactions") {
        PeriodicBoundaryConditions pbc(10);

        // Particles look and are placed like this (x - central particle, o - second one):
        //
        //       1   4   6   9
        //   ||  x-->o   o<--x  ||
        //
        // 4, 6 should be tangent on scale 10. Each particle should be reflected separately! If all are reflected
        // simultaneously, than they are no longer tangent

        CHECK(interaction.hasHardPart());
        CHECK_FALSE(interaction.hasSoftPart());
        CHECK(interaction.getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}, {3, 0, 0}});

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
}

TEST_CASE("PolysphereTraits: printers") {
    PolysphereGeometry geometry({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}}, {1, 0, 0}, {0, 1, 0}, {0.5, 0, 0});
    PolysphereTraits traits(std::move(geometry));

    Shape shape({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0));
    std::string expected = "{Sphere[{9.000000, 5.000000, 5.000000},0.500000],"
                           "Sphere[{6.000000, 5.000000, 5.000000},1.000000]}";
    CHECK(traits.getPrinter("wolfram", {})->print(shape) == expected);
}
