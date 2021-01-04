//
// Created by Piotr Kubala on 23/12/2020.
//

#include <catch2/catch.hpp>

#include "core/shapes/PolysphereTraits.h"
#include "core/PeriodicBoundaryConditions.h"

namespace {
    class DummyInteraction : public CentralInteraction {
    protected:
        [[nodiscard]] double calculateEnergyForDistance2([[maybe_unused]] double distance) const override { return 0; }
    };
}

TEST_CASE("PolysphereTraits: hard interactions") {
    PolysphereTraits traits({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}});

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

    SECTION("toWolfram") {
        Shape shape({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0));

        std::string expected = "{Sphere[{9.000000, 5.000000, 5.000000},0.500000],"
                               "Sphere[{6.000000, 5.000000, 5.000000},1.000000]}";
        CHECK(traits.getPrinter().toWolfram(shape) == expected);
    }

    SECTION("getVolume") {
        CHECK(traits.getVolume() == Approx(4.71238898038469));
    }
}

TEST_CASE("PolysphereTraits: soft interactions") {
    PolysphereTraits traits({{{0, 0, 0}, 0.5}, {{3, 0, 0}, 1}},
                            std::make_unique<DummyInteraction>());
    const auto &interaction = dynamic_cast<const CentralInteraction &>(traits.getInteraction());

    CHECK(interaction.getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}, {3, 0, 0}});
}