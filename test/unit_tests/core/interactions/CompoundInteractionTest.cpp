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
    ALLOW_CALL(name, hasWallPart()).RETURN(false); \
    ALLOW_CALL(name, isConvex()).RETURN(false); \
    ALLOW_CALL(name, getRangeRadius(_)).RETURN(0); \
    ALLOW_CALL(name, getTotalRangeRadius(_)).RETURN(0); \
    ALLOW_CALL(name, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{})

#define MAKE_SOFT_MOCK(name) \
    MockInteraction name; \
    ALLOW_CALL(name, hasHardPart()).RETURN(false); \
    ALLOW_CALL(name, hasSoftPart()).RETURN(true); \
    ALLOW_CALL(name, hasWallPart()).RETURN(false); \
    ALLOW_CALL(name, isConvex()).RETURN(false); \
    ALLOW_CALL(name, getRangeRadius(_)).RETURN(0); \
    ALLOW_CALL(name, getTotalRangeRadius(_)).RETURN(0); \
    ALLOW_CALL(name, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{})

#define MAKE_WALL_MOCK(name) \
    MockInteraction name; \
    ALLOW_CALL(name, hasHardPart()).RETURN(false); \
    ALLOW_CALL(name, hasSoftPart()).RETURN(false); \
    ALLOW_CALL(name, hasWallPart()).RETURN(true); \
    ALLOW_CALL(name, isConvex()).RETURN(false); \
    ALLOW_CALL(name, getRangeRadius(_)).RETURN(0); \
    ALLOW_CALL(name, getTotalRangeRadius(_)).RETURN(0); \
    ALLOW_CALL(name, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{})


using trompeloeil::_;


// TODO: test CompoundInteraction with shape data

TEST_CASE("CompoundInteraction") {
    MAKE_HARD_MOCK(hard);
    MAKE_SOFT_MOCK(soft);
    MAKE_WALL_MOCK(wall);

    SECTION("range radius") {
        ALLOW_CALL(hard, getRangeRadius(_)).RETURN(1);
        ALLOW_CALL(soft, getRangeRadius(_)).RETURN(2);
        ALLOW_CALL(hard, getTotalRangeRadius(_)).RETURN(3);
        ALLOW_CALL(soft, getTotalRangeRadius(_)).RETURN(4);
        CompoundInteraction compound(hard, soft);

        CHECK(compound.getRangeRadius(nullptr) == 2);
        CHECK(compound.getTotalRangeRadius(nullptr) == 4);
    }

    SECTION("interaction centres") {
        SECTION("0 centres") {
            ALLOW_CALL(hard, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{});
            ALLOW_CALL(soft, getInteractionCentres(_)).RETURN(std::vector<Vector<3>>{});
            CompoundInteraction compound(hard, soft);

            CHECK(compound.getInteractionCentres(nullptr).empty());
        }

        SECTION("2 centres") {
            std::vector<Vector<3>> centres{{1, 2, 3}, {4, 5, 6}};
            ALLOW_CALL(hard, getInteractionCentres(_)).RETURN(centres);
            ALLOW_CALL(soft, getInteractionCentres(_)).RETURN(centres);
            CompoundInteraction compound(hard, soft);

            CHECK(compound.getInteractionCentres(nullptr) == centres);
        }

        /*SECTION("incompatible") {
            std::vector<Vector<3>> centres1{{1, 2, 3}, {4, 5, 6}};
            std::vector<Vector<3>> centres2{{7, 8, 9}, {10, 11, 12}};
            std::vector<Vector<3>> emptyCentres{};

            SECTION("different number") {
                ALLOW_CALL(hard, getInteractionCentres(_)).RETURN(centres1);
                ALLOW_CALL(soft, getInteractionCentres(_)).RETURN(emptyCentres);

                CHECK_THROWS(CompoundInteraction(soft, hard));
            }

            SECTION("same number buf unequal") {
                ALLOW_CALL(hard, getInteractionCentres(_)).RETURN(centres1);
                ALLOW_CALL(soft, getInteractionCentres(_)).RETURN(centres2);

                CHECK_THROWS(CompoundInteraction(soft, hard));
            }
        }*/
    }

    SECTION("has...Part") {
        auto [int1, int2, name1, name2, hasSoft, hasHard, hasWall]
            = GENERATE_REF(std::make_tuple(&hard, &hard, "hard", "hard", false, true, false),
                           std::make_tuple(&soft, &soft, "soft", "soft", true, false, false),
                           std::make_tuple(&wall, &wall, "wall", "wall", false, false, true),
                           std::make_tuple(&hard, &soft, "hard", "soft", true, true, false),
                           std::make_tuple(&soft, &wall, "soft", "wall", true, false, true),
                           std::make_tuple(&wall, &hard, "wall", "hard", false, true, true));

        DYNAMIC_SECTION(name1 << " + " << name2) {
            CompoundInteraction compound(*int1, *int2);

            CHECK(compound.hasSoftPart() == hasSoft);
            CHECK(compound.hasHardPart() == hasHard);
            CHECK(compound.hasWallPart() == hasWall);
        }
    }

    SECTION("convexity behaviour") {
        MAKE_HARD_MOCK(hardConvex);
        ALLOW_CALL(hardConvex, isConvex()).RETURN(true);
        MAKE_HARD_MOCK(hardConcave);
        ALLOW_CALL(hardConcave, isConvex()).RETURN(false);
        auto [int1, int2, name1, name2, isConvex]
                = GENERATE_REF(std::make_tuple(&hardConvex, &hardConvex, "hard convex", "hard convex", false),
                               std::make_tuple(&hardConvex, &soft, "hard convex", "not hard", true),
                               std::make_tuple(&hardConcave, &soft, "hard concave", "not hard", false));

        DYNAMIC_SECTION(name1 << " + " << name2) {
            CompoundInteraction compound1(*int1, *int2);
            CompoundInteraction compound2(*int2, *int1);

            CHECK(compound1.isConvex() == isConvex);
            CHECK(compound2.isConvex() == isConvex);
        }
    }

    Vector<3> pos1{1, 2, 3}, pos2{4, 5, 6};
    Matrix<3, 3> rot1{1, 0, 0, 0, 1, 0, 0, 0, 1}, rot2{0, 1, 0, 0, 0, 1, 1, 0, 0};
    std::size_t idx1{1}, idx2{2};
    Vector<3> wallOrigin{0, 0, 0};
    Vector<3> wallVector{0, 0, 1};
    PeriodicBoundaryConditions bc;

    bool ov1{}, ov2{}, result{};

    SECTION("hard + soft overlap and energy") {
        REQUIRE_CALL(hard, overlapBetween(pos1, rot1, _, idx1, pos2, rot2, _, idx2, _)).RETURN(true);
        REQUIRE_CALL(soft, calculateEnergyBetween(pos1, rot1, _, idx1, pos2, rot2, _, idx2, _)).RETURN(2);
        CompoundInteraction compound(hard, soft);

        CHECK(compound.overlapBetween(pos1, rot1, nullptr, idx1, pos2, rot2, nullptr, idx2, bc));
        CHECK(compound.calculateEnergyBetween(pos1, rot1, nullptr, idx1, pos2, rot2, nullptr, idx2, bc) == 2);
    }

    SECTION("hard + hard overlap") {
        MAKE_HARD_MOCK(hard2);

        std::tie(ov1, ov2, result) = GENERATE(std::make_tuple(false, false, false),
                                              std::make_tuple(false, true, true),
                                              std::make_tuple(true, false, true),
                                              std::make_tuple(true, true, true));

        DYNAMIC_SECTION(std::boolalpha << ov1 << " and " << ov2 << " give " << result) {
            ALLOW_CALL(hard, overlapBetween(pos1, rot1, _, idx1, pos2, rot2, _, idx2, _)).RETURN(ov1);
            ALLOW_CALL(hard2, overlapBetween(pos1, rot1, _, idx1, pos2, rot2, _, idx2, _)).RETURN(ov2);
            CompoundInteraction compound(hard, hard2);

            CHECK(compound.overlapBetween(pos1, rot1, nullptr, idx1, pos2, rot2, nullptr, idx2, bc) == result);
        }
    }

    SECTION("wall + wall overlap") {
        MAKE_WALL_MOCK(wall2);

        std::tie(ov1, ov2, result) = GENERATE(std::make_tuple(false, false, false),
                                              std::make_tuple(false, true, true),
                                              std::make_tuple(true, false, true),
                                              std::make_tuple(true, true, true));

        DYNAMIC_SECTION(std::boolalpha << ov1 << " and " << ov2 << " give " << result) {
            ALLOW_CALL(wall, overlapWithWall(pos1, rot1, _, idx1, wallOrigin, wallVector)).RETURN(ov1);
            ALLOW_CALL(wall2, overlapWithWall(pos1, rot1, _, idx1, wallOrigin, wallVector)).RETURN(ov2);
            CompoundInteraction compound(wall, wall2);

            CHECK(compound.overlapWithWall(pos1, rot1, nullptr, idx1, wallOrigin, wallVector) == result);
        }
    }

    SECTION("soft + soft energy") {
        MAKE_SOFT_MOCK(soft2);

        REQUIRE_CALL(soft, calculateEnergyBetween(pos1, rot1, _, idx1, pos2, rot2, _, idx2, _)).RETURN(1);
        REQUIRE_CALL(soft2, calculateEnergyBetween(pos1, rot1, _, idx1, pos2, rot2, _, idx2, _)).RETURN(2);
        CompoundInteraction compound(soft, soft2);

        CHECK(compound.calculateEnergyBetween(pos1, rot1, nullptr, idx1, pos2, rot2, nullptr, idx2, bc) == 3);
    }
}