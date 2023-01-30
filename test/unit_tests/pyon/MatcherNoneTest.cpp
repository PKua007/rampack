//
// Created by Piotr Kubala on 25/12/2022.
//

#include <catch2/catch.hpp>

#include "matchers/UnmatchedWithReason.h"

#include "pyon/MatcherNone.h"
#include "pyon/Parser.h"

using namespace pyon;
using namespace pyon::ast;
using namespace pyon::matcher;


TEST_CASE("Matcher: None") {
    Any result;

    SECTION("default map") {
        MatcherNone matcher;

        CHECK_THAT(matcher.match(Parser::parse("45"), result),
                   UnmatchedWithReason(R"(Matching None failed:
✖ Got incorrect node type: Integer
✓ Expected format: None)"));

        CHECK(matcher.match(Parser::parse("None"), result));
        CHECK(result.isEmpty());
        CHECK(matcher.outline(4) == "    None");
    }

    SECTION("synopsis") {
        CHECK(MatcherNone{}.synopsis() == "None");
    }

    SECTION("custom map") {
        auto matcher = MatcherNone{}.mapTo([]() -> int { return 45; } );
        CHECK(matcher.match(Parser::parse("None"), result));
        CHECK(result.as<int>() == 45);
    }

    SECTION("default construction map") {
        auto matcher = MatcherNone{}.mapTo<std::optional<int>>();
        CHECK(matcher.match(Parser::parse("None"), result));
        CHECK(result.as<std::optional<int>>() == std::nullopt);
    }
}