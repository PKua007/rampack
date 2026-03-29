//
// Created by Codex on 29/03/2026.
//

#include <catch2/catch.hpp>

#include "core/interactions/InteractionCentreLayout.h"
#include "utils/Exceptions.h"

TEST_CASE("InteractionCentreLayout: construction") {
    SECTION("valid construction") {
        SECTION("default constructor creates sphere layout") {
            InteractionCentreLayout layout;

            CHECK(layout.getCentres().empty());
            CHECK(layout.getCentreIdxTypeMap() == std::vector<std::size_t>{0});
            CHECK(layout.numCentres() == 0);
            CHECK(layout.numCentreTypes() == 1);
        }

        SECTION("explicit constructor creates sphere layout") {
            InteractionCentreLayout layout({}, {0});

            CHECK(layout.getCentres().empty());
            CHECK(layout.getCentreIdxTypeMap() == std::vector<std::size_t>{0});
            CHECK(layout.numCentres() == 0);
            CHECK(layout.numCentreTypes() == 1);
        }

        SECTION("general layout") {
            InteractionCentreLayout layout({{0, 0, 0}, {1, 0, 0}}, {2, 0});

            CHECK(layout.getCentres() == std::vector<Vector<3>>{{0, 0, 0}, {1, 0, 0}});
            CHECK(layout.getCentreIdxTypeMap() == std::vector<std::size_t>{2, 0});
            CHECK(layout.numCentres() == 2);
            CHECK(layout.numCentreTypes() == 3);
        }
    }

    SECTION("invalid construction") {
        SECTION("mismatched centres and mapping sizes") {
            CHECK_THROWS_AS(InteractionCentreLayout({{0, 0, 0}}, {0, 1}), PreconditionException);
        }

        SECTION("empty centre type mapping for non-empty centres") {
            CHECK_THROWS_AS(InteractionCentreLayout({{0, 0, 0}}, {}), PreconditionException);
        }

        SECTION("non-canonical empty centre layout") {
            CHECK_THROWS_AS(InteractionCentreLayout({}, {}), PreconditionException);
            CHECK_THROWS_AS(InteractionCentreLayout({}, {1}), PreconditionException);
            CHECK_THROWS_AS(InteractionCentreLayout({}, {0, 0}), PreconditionException);
        }
    }
}

TEST_CASE("InteractionCentreLayout: operations") {
    SECTION("equality compares centres and mapping") {
        InteractionCentreLayout original({{0, 0, 0}, {1, 0, 0}}, {0, 1});
        InteractionCentreLayout identical({{0, 0, 0}, {1, 0, 0}}, {0, 1});
        InteractionCentreLayout differentCentres({{0, 0, 0}, {2, 0, 0}}, {0, 1});
        InteractionCentreLayout differentMapping({{0, 0, 0}, {1, 0, 0}}, {0, 0});

        CHECK(original == identical);
        CHECK_FALSE(original != identical);
        CHECK(original != differentCentres);
        CHECK(original != differentMapping);
    }

    SECTION("reports cached layout properties") {
        InteractionCentreLayout layout({{0, 0, 0}, {1, 0, 0}}, {2, 0});

        CHECK(layout.numCentres() == 2);
        CHECK(layout.numCentreTypes() == 3);
    }
}
