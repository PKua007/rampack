//
// Created by pkua on 05.03.2022.
//

#include <catch2/catch.hpp>

#include "mocks/MockInteraction.h"

#include "core/interactions/CompoundInteraction.h"
#include "core/PeriodicBoundaryConditions.h"


#define MAKE_HARD_MOCK(name) \
    MockInteraction name; \
    ALLOW_CALL(name, hasHardPart()).RETURN(true); \
    ALLOW_CALL(name, hasSoftPart()).RETURN(false); \
    ALLOW_CALL(name, getRangeRadius()).RETURN(0); \
    ALLOW_CALL(name, getTotalRangeRadius()).RETURN(0); \
    ALLOW_CALL(name, getInteractionCentres()).RETURN(std::vector<Vector<3>>{})

#define MAKE_SOFT_MOCK(name) \
    MockInteraction name; \
    ALLOW_CALL(name, hasHardPart()).RETURN(false); \
    ALLOW_CALL(name, hasSoftPart()).RETURN(true); \
    ALLOW_CALL(name, getRangeRadius()).RETURN(0); \
    ALLOW_CALL(name, getTotalRangeRadius()).RETURN(0); \
    ALLOW_CALL(name, getInteractionCentres()).RETURN(std::vector<Vector<3>>{})


using trompeloeil::_;


TEST_CASE("CompoundInteraction") {
    MAKE_HARD_MOCK(hard);
    MAKE_SOFT_MOCK(soft);

    SECTION("range radius") {
        ALLOW_CALL(hard, getRangeRadius()).RETURN(1);
        ALLOW_CALL(soft, getRangeRadius()).RETURN(2);
        ALLOW_CALL(hard, getTotalRangeRadius()).RETURN(3);
        ALLOW_CALL(soft, getTotalRangeRadius()).RETURN(4);
        CompoundInteraction compound(hard, soft);

        CHECK(compound.getRangeRadius() == 2);
        CHECK(compound.getTotalRangeRadius() == 4);
    }

    SECTION("interaction centres") {
        SECTION("0 centres") {
            ALLOW_CALL(hard, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
            ALLOW_CALL(soft, getInteractionCentres()).RETURN(std::vector<Vector<3>>{});
            CompoundInteraction compound(hard, soft);

            CHECK(compound.getInteractionCentres().empty());
        }

        SECTION("2 centres") {
            std::vector<Vector<3>> centres{{1, 2, 3}, {4, 5, 6}};
            ALLOW_CALL(hard, getInteractionCentres()).RETURN(centres);
            ALLOW_CALL(soft, getInteractionCentres()).RETURN(centres);
            CompoundInteraction compound(hard, soft);

            CHECK(compound.getInteractionCentres() == centres);
        }

        SECTION("incompatible") {
            std::vector<Vector<3>> centres1{{1, 2, 3}, {4, 5, 6}};
            std::vector<Vector<3>> centres2{{7, 8, 9}, {10, 11, 12}};
            std::vector<Vector<3>> emptyCentres{};

            SECTION("different number") {
                ALLOW_CALL(hard, getInteractionCentres()).RETURN(centres1);
                ALLOW_CALL(soft, getInteractionCentres()).RETURN(emptyCentres);

                CHECK_THROWS(CompoundInteraction(soft, hard));
            }

            SECTION("same number buf unequal") {
                ALLOW_CALL(hard, getInteractionCentres()).RETURN(centres1);
                ALLOW_CALL(soft, getInteractionCentres()).RETURN(centres2);

                CHECK_THROWS(CompoundInteraction(soft, hard));
            }
        }
    }

    SECTION("has...Part") {
        SECTION("hard + hard") {
            CompoundInteraction compound(hard, hard);

            CHECK_FALSE(compound.hasSoftPart());
            CHECK(compound.hasHardPart());
        }

        SECTION("soft + soft") {
            CompoundInteraction compound(soft, soft);

            CHECK(compound.hasSoftPart());
            CHECK_FALSE(compound.hasHardPart());
        }

        SECTION("hard + soft") {
            CompoundInteraction compound(hard, soft);

            CHECK(compound.hasSoftPart());
            CHECK(compound.hasHardPart());
        }
    }

    Vector<3> pos1{1, 2, 3}, pos2{4, 5, 6};
    Matrix<3, 3> rot1{1, 0, 0, 0, 1, 0, 0, 0, 1}, rot2{0, 1, 0, 0, 0, 1, 1, 0, 0};
    std::size_t idx1{1}, idx2{2};
    PeriodicBoundaryConditions bc;

    SECTION("hard + soft overlap and energy") {
        REQUIRE_CALL(hard, overlapBetween(pos1, rot1, idx1, pos2, rot2, idx2, _)).RETURN(true);
        REQUIRE_CALL(soft, calculateEnergyBetween(pos1, rot1, idx1, pos2, rot2, idx2, _)).RETURN(2);
        CompoundInteraction compound(hard, soft);

        CHECK(compound.overlapBetween(pos1, rot1, idx1, pos2, rot2, idx2, bc));
        CHECK(compound.calculateEnergyBetween(pos1, rot1, idx1, pos2, rot2, idx2, bc) == 2);
    }

    SECTION("hard + hard overlap") {
        MAKE_HARD_MOCK(hard2);

        auto [ov1, ov2, result] = GENERATE(std::make_tuple(false, false, false),
                                           std::make_tuple(false, true, true),
                                           std::make_tuple(true, false, true),
                                           std::make_tuple(true, true, true));

        DYNAMIC_SECTION(std::boolalpha << ov1 << " and " << ov2 << " give " << result) {
            ALLOW_CALL(hard, overlapBetween(pos1, rot1, idx1, pos2, rot2, idx2, _)).RETURN(ov1);
            ALLOW_CALL(hard2, overlapBetween(pos1, rot1, idx1, pos2, rot2, idx2, _)).RETURN(ov2);
            CompoundInteraction compound(hard, hard2);

            CHECK(compound.overlapBetween(pos1, rot1, idx1, pos2, rot2, idx2, bc) == result);
        }
    }

    SECTION("soft + soft energy") {
        MAKE_SOFT_MOCK(soft2);

        REQUIRE_CALL(soft, calculateEnergyBetween(pos1, rot1, idx1, pos2, rot2, idx2, _)).RETURN(1);
        REQUIRE_CALL(soft2, calculateEnergyBetween(pos1, rot1, idx1, pos2, rot2, idx2, _)).RETURN(2);
        CompoundInteraction compound(soft, soft2);

        CHECK(compound.calculateEnergyBetween(pos1, rot1, idx1, pos2, rot2, idx2, bc) == 3);
    }
}