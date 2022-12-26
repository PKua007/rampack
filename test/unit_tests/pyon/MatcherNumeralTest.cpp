//
// Created by Piotr Kubala on 25/12/2022.
//

#include <catch2/catch.hpp>

#include "matchers/UnmatchedWithReason.h"

#include "pyon/MatcherNumeral.h"
#include "pyon/Parser.h"

using namespace pyon;
using namespace pyon::ast;
using namespace pyon::matcher;


TEST_CASE("Matcher: Int") {
    Any result;

    SECTION("default") {
        CHECK_THAT(MatcherInt{}.match(Parser::parse("True"), result),
                   UnmatchedWithReason(R"(Matching Integer failed:
✖ Got incorrect node type: Boolean
✓ Expected format: Integer)"));

        CHECK(MatcherInt{}.match(Parser::parse("7"), result));
        CHECK(result.as<long>() == 7);
        CHECK(MatcherInt{}.outline(4) == "    Integer");
    }

    SECTION("filters") {
        SECTION("positive") {
            auto matcher = MatcherInt{}.positive();

            CHECK_THAT(matcher.match(Parser::parse("0"), result),
                       UnmatchedWithReason(R"(Matching Integer failed:
✖ Condition not satisfied: > 0
✓ Expected format: Integer, > 0)"));

            CHECK(matcher.match(Parser::parse("7"), result));
            CHECK(matcher.outline(4) == "    Integer, > 0");
        }

        SECTION("negative") {
            auto matcher = MatcherInt{}.negative();
            CHECK_FALSE(matcher.match(Parser::parse("0"), result));
            CHECK(matcher.match(Parser::parse("-7"), result));
            CHECK(matcher.outline(4) == "    Integer, < 0");
        }

        SECTION("non-positive") {
            auto matcher = MatcherInt{}.nonPositive();
            CHECK_FALSE(matcher.match(Parser::parse("1"), result));
            CHECK(matcher.match(Parser::parse("0"), result));
            CHECK(matcher.match(Parser::parse("-1"), result));
            CHECK(matcher.outline(4) == "    Integer, <= 0");
        }

        SECTION("non-negative") {
            auto matcher = MatcherInt{}.nonNegative();
            CHECK_FALSE(matcher.match(Parser::parse("-1"), result));
            CHECK(matcher.match(Parser::parse("0"), result));
            CHECK(matcher.match(Parser::parse("1"), result));
            CHECK(matcher.outline(4) == "    Integer, >= 0");
        }

        SECTION("greater") {
            auto matcher = MatcherInt{}.greater(5);
            CHECK_FALSE(matcher.match(Parser::parse("5"), result));
            CHECK(matcher.match(Parser::parse("6"), result));
            CHECK(matcher.outline(4) == "    Integer, > 5");
        }

        SECTION("greater equals") {
            auto matcher = MatcherInt{}.greaterEquals(5);
            CHECK_FALSE(matcher.match(Parser::parse("4"), result));
            CHECK(matcher.match(Parser::parse("5"), result));
            CHECK(matcher.outline(4) == "    Integer, >= 5");
        }

        SECTION("less") {
            auto matcher = MatcherInt{}.less(5);
            CHECK_FALSE(matcher.match(Parser::parse("5"), result));
            CHECK(matcher.match(Parser::parse("4"), result));
            CHECK(matcher.outline(4) == "    Integer, < 5");
        }

        SECTION("less equals") {
            auto matcher = MatcherInt{}.lessEquals(5);
            CHECK_FALSE(matcher.match(Parser::parse("6"), result));
            CHECK(matcher.match(Parser::parse("5"), result));
            CHECK(matcher.outline(4) == "    Integer, <= 5");
        }

        SECTION("equals") {
            auto matcher1 = MatcherInt{}.equals(5);
            auto matcher2 = MatcherInt(5);
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse("4"), result));
            CHECK_FALSE(matcher.match(Parser::parse("6"), result));
            CHECK(matcher.match(Parser::parse("5"), result));
            CHECK(matcher.outline(4) == "    Integer, = 5");
        }

        SECTION("any of") {
            auto matcher1 = MatcherInt{}.anyOf({1, 3, 5});
            auto matcher2 = MatcherInt{1, 3, 5};
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse("2"), result));
            CHECK(matcher.match(Parser::parse("1"), result));
            CHECK(matcher.match(Parser::parse("5"), result));
            CHECK(matcher.outline(4) == "    Integer, one of: 1, 3, 5");
        }

        SECTION("in range") {
            auto matcher = MatcherInt{}.inRange(1, 3);
            CHECK_FALSE(matcher.match(Parser::parse("0"), result));
            CHECK_FALSE(matcher.match(Parser::parse("4"), result));
            CHECK(matcher.match(Parser::parse("1"), result));
            CHECK(matcher.match(Parser::parse("3"), result));
            CHECK(matcher.outline(4) == "    Integer, in range [1, 3]");
        }

        SECTION("custom") {
            auto matcher = MatcherInt{}.filter([](long i) { return i % 2 == 0; });
            CHECK_FALSE(matcher.match(Parser::parse("3"), result));
            CHECK(matcher.match(Parser::parse("4"), result));
            CHECK(matcher.outline(4) == "    Integer, <undefined filter>");
            matcher.describe("even");
            CHECK(matcher.outline(4) == "    Integer, even");
        }

        SECTION("joined") {
            auto matcher = MatcherInt{}
                    .greater(5)
                    .filter([](long i) { return i % 2 == 0; })
                    .describe("even");
            CHECK_FALSE(matcher.match(Parser::parse("4"), result));
            CHECK_FALSE(matcher.match(Parser::parse("7"), result));
            CHECK(matcher.match(Parser::parse("8"), result));
            CHECK(matcher.outline(4) == "    Integer:\n    - > 5\n    - even");
        }
    }

    SECTION("map to") {
        auto matcher = MatcherInt{}.mapTo<std::size_t>();
        CHECK(matcher.match(Parser::parse("5"), result));
        CHECK(result.as<std::size_t>() == 5ul);
    }

    SECTION("custom map") {
        auto matcher = MatcherInt{}.mapTo([](long i) { return std::string(static_cast<std::size_t>(i), 'a'); });
        CHECK(matcher.match(Parser::parse("5"), result));
        CHECK(result.as<std::string>() == "aaaaa");
    }
}

TEST_CASE("Matcher: Float") {
    // only basic, since it is templatized and tested on int
    Any result;

    CHECK_THAT(MatcherFloat{}.match(Parser::parse("True"), result),
               UnmatchedWithReason(R"(Matching Float failed:
✖ Got incorrect node type: Boolean
✓ Expected format: Float)"));

    REQUIRE(MatcherFloat{}.match(Parser::parse("7.5"), result));
    CHECK(result.as<double>() == 7.5);

    REQUIRE(MatcherFloat{}.match(Parser::parse("7"), result));
    CHECK(result.as<double>() == 7);
}