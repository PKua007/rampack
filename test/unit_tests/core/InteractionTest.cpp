//
// Created by Piotr Kubala on 13/03/2021.
//

#include <catch2/catch.hpp>
#include <utility>

#include "core/Interaction.h"

namespace {
    class DummyInteraction : public Interaction {
    private:
        double rangeRadius{};
        std::vector<Vector<3>> interactionCentres;

    public:
        DummyInteraction(double rangeRadius, std::vector<Vector<3>> interactionCentres)
                : rangeRadius{rangeRadius}, interactionCentres{std::move(interactionCentres)}
        { }

        [[nodiscard]] double getRangeRadius() const override { return this->rangeRadius; }
        [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override { return this->interactionCentres; }
        [[nodiscard]] bool hasHardPart() const override { return false; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }
        [[nodiscard]] bool hasWallPart() const override { return false; }
        [[nodiscard]] bool isConvex() const override { return false; }
    };
}

TEST_CASE("Interaction: totalRangeRadius") {
    // One interaction centre is distant by 5 from the origin, the second by a smaller value (sqrt(6) or what not)
    DummyInteraction dummyInteraction(3, {{0, 3, 4}, {1, -1, 2}});

    double totalRangeRadius = dummyInteraction.getTotalRangeRadius();

    CHECK(totalRangeRadius == Approx(13));  // 5 + 5 + 3
}