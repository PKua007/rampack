//
// Created by Piotr Kubala on 19/12/2022.
//

#include <catch2/catch.hpp>

#include "core/shapes/KMerTraits.h"


TEST_CASE("KMer: volume") {
    SECTION("non-overlapping") {
        KMerTraits traits(3, 0.5, 1);
        CHECK(traits.getGeometry().getVolume() == Approx(3. * 4/3*M_PI * 0.5*0.5*0.5));
    }

    SECTION("overlapping") {
        KMerTraits traits(3, 0.5, 0.5);
        CHECK(traits.getGeometry().getVolume() == Approx(19.*M_PI/48));  // Mathematica value
    }
}

TEST_CASE("KMer: interaction centre layout") {
    KMerTraits traits(3, 0.5, 1);
    const auto &geometry = dynamic_cast<const PolysphereTraits::PolysphereGeometry &>(traits.getGeometry());

    CHECK(geometry.getInteractionCentreLayout().getCentreIdxTypeIdxMap() == std::vector<std::size_t>{0, 0, 0});
    CHECK(geometry.getDisplayRadiiByType() == std::vector<double>{0.5});
}
