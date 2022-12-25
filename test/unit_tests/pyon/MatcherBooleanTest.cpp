//
// Created by Piotr Kubala on 25/12/2022.
//

#include <catch2/catch.hpp>

#include "matchers/UnmatchedWithReason.h"

#include "pyon/Matcher.h"
#include "pyon/Parser.h"

using namespace pyon;
using namespace pyon::ast;
using namespace pyon::matcher;


TEST_CASE("Matcher: Boolean") {
    Any result;

    SECTION("default") {
        CHECK_THAT(MatcherBoolean{}.match(Parser::parse("56"), result),
                   UnmatchedWithReason(R"(Matching Boolean failed:
✖ Got incorrect node type: Integer
✓ Expected format: Boolean)"));

        CHECK(MatcherBoolean{}.match(Parser::parse("True"), result));
        CHECK(result.as<bool>());
        CHECK(MatcherBoolean{}.match(Parser::parse("False"), result));
        CHECK_FALSE(result.as<bool>());
        CHECK(MatcherBoolean{}.outline(4) == "    Boolean");
    }

    SECTION("filters") {
        SECTION("true") {
            auto matcher1 = MatcherBoolean{true};
            auto matcher2 = MatcherBoolean{}.isTrue();
            auto matcher = GENERATE_COPY(matcher1, matcher2);

            CHECK_THAT(matcher.match(Parser::parse("False"), result),
                       UnmatchedWithReason(R"(Matching Boolean failed:
✖ Boolean is False
✓ Expected format: Boolean equal

True)"));

            CHECK(matcher.match(Parser::parse("True"), result));
            CHECK(matcher.outline(4) == "    Boolean equal True");
        }

        SECTION("false") {
            auto matcher1 = MatcherBoolean{false};
            auto matcher2 = MatcherBoolean{}.isFalse();
            auto matcher = GENERATE_COPY(matcher1, matcher2);

            CHECK_THAT(matcher.match(Parser::parse("True"), result),
                       UnmatchedWithReason(R"(Matching Boolean failed:
✖ Boolean is True
✓ Expected format: Boolean equal False)"));

            CHECK(matcher.match(Parser::parse("False"), result));
            CHECK(matcher.outline(4) == "    Boolean equal False");
        }
    }

    SECTION("map to") {
        auto matcher = MatcherBoolean{}.mapTo<std::size_t>();
        CHECK(matcher.match(Parser::parse("True"), result));
        CHECK(result.as<std::size_t>() == 1ul);
    }

    SECTION("custom map") {
        auto matcher = MatcherBoolean{}.mapTo([](bool b) -> std::string { return b ? "true" : "false"; });
        CHECK(matcher.match(Parser::parse("True"), result));
        CHECK(result.as<std::string>() == "true");
    }
}