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


TEST_CASE("Matcher: Alternative") {
    Any result;

    SECTION("2 alternatives") {
        auto matcher = MatcherInt{} | MatcherFloat{};
        CHECK_FALSE(matcher.match(Parser::parse("True"), result));
        CHECK(matcher.match(Parser::parse("1"), result));
        CHECK(result.as<long>() == 1);
        CHECK(matcher.match(Parser::parse("1.2"), result));
        CHECK(result.as<double>() == 1.2);
    }

    SECTION("3 alternatives") {
        auto matcher = MatcherInt{} | MatcherFloat{} | MatcherString{};
        CHECK_FALSE(matcher.match(Parser::parse("True"), result));
        CHECK(matcher.match(Parser::parse("1"), result));
        CHECK(result.as<long>() == 1);
        CHECK(matcher.match(Parser::parse("1.2"), result));
        CHECK(result.as<double>() == 1.2);
        CHECK(matcher.match(Parser::parse(R"("abc")"), result));
        CHECK(result.as<std::string>() == "abc");
    }

    SECTION("matching order") {
        auto matcher1 = MatcherInt{}.mapTo([](const auto&) -> int { return 1; });
        auto matcher2 = MatcherInt{}.mapTo([](const auto&) -> int { return 2; });
        auto matcher3 = MatcherInt{}.mapTo([](const auto&) -> int { return 3; });

        SECTION("1 | (2 | 3)") {
            auto matcher23 = matcher2 | matcher3;
            auto matcher123 = matcher1 | matcher23;
            REQUIRE(matcher123.match(Parser::parse("1337"), result));
            CHECK(result.as<int>() == 1);
        }

        SECTION("(1 | 2) | 3") {
            auto matcher12 = matcher1 | matcher2;
            auto matcher123 = matcher12 | matcher3;
            REQUIRE(matcher123.match(Parser::parse("1337"), result));
            CHECK(result.as<int>() == 1);
        }
    }

    SECTION("joining alternatives with dataclasses") {
        auto matcher12 = MatcherDataclass("class1") | MatcherDataclass("class2");
        auto matcher34 = MatcherDataclass("class3") | MatcherDataclass("class4");
        auto matcher1234 = matcher12 | matcher34;

        REQUIRE(matcher1234.match(Parser::parse("class1"), result));
        REQUIRE(matcher1234.match(Parser::parse("class2"), result));
        REQUIRE(matcher1234.match(Parser::parse("class3"), result));
        REQUIRE(matcher1234.match(Parser::parse("class4"), result));
    }

    SECTION("synopsis") {
        SECTION("empty") {
            MatcherAlternative matcher;
            REQUIRE(matcher.synopsis() == "(empty Alternative)");
        }

        SECTION("non-empty") {
            auto matcher = MatcherDataclass("class1") | MatcherInt{} | MatcherDataclass("class1") | MatcherString{};
            REQUIRE(matcher.synopsis() == R"(class "class1" (2 variants) | Integer | String)");
        }
    }

    SECTION("outline") {
        SECTION("layout") {
            auto matcher = MatcherInt{}.positive().lessEquals(5) | MatcherFloat{}.positive() | MatcherString{};
            CHECK(matcher.outline(4) == R"(    Alternative:
    1. Integer:
       - > 0
       - <= 5
    2. Float, > 0
    3. String)");
        }

        SECTION("number indentation") {
            MatcherAlternative matcher;
            for (std::size_t i{}; i < 10; i++)
                matcher |= MatcherInt{};
            CHECK(matcher.outline(4) == R"(    Alternative:
    1.  Integer
    2.  Integer
    3.  Integer
    4.  Integer
    5.  Integer
    6.  Integer
    7.  Integer
    8.  Integer
    9.  Integer
    10. Integer)");
        }
    }

    SECTION("error reporting") {
        SECTION("unmatched node") {
            auto matcher = MatcherInt{}.positive().less(5)
                | MatcherString{}
                | MatcherArray{}.elementsMatch(MatcherInt{})
                | MatcherArray{}.elementsMatch(MatcherString{})
                | MatcherString{};
            CHECK_THAT(matcher.match(Parser::parse("True"), result),
                       UnmatchedWithReason(R"(Matching Alternative failed:
✖ Got incorrect node type: Boolean
✓ Available alternatives:
  1. Integer
  2. String (2 variants)
  3. Array[Integer]
  4. Array[String])"));
        }

        SECTION("matched node error") {
            SECTION("1 variant") {
                auto matcher = MatcherInt{}.positive().less(5) | MatcherString{};
                CHECK_THAT(matcher.match(Parser::parse("6"), result),
                           UnmatchedWithReason(R"(Matching Integer failed:
✖ Condition not satisfied: < 5)"));
            }

            SECTION("2 variants") {
                auto matcher = MatcherInt{}.positive() | MatcherInt{}.negative() | MatcherString{};
                CHECK_THAT(matcher.match(Parser::parse("0"), result),
                           UnmatchedWithReason(
R"(Matching Alternative failed: all 2 variants of Integer failed to match:
✖ (variant 1) Matching Integer failed:
  ✖ Condition not satisfied: > 0
✖ (variant 2) Matching Integer failed:
  ✖ Condition not satisfied: < 0)"));
            }
        }

        SECTION("unmatched class name") {
            auto matcher = MatcherNone{} | MatcherDataclass("class");
            CHECK_THAT(matcher.match(Parser::parse("class2"), result),
                       UnmatchedWithReason(R"(Matching Alternative failed:
✖ Got unknown class: "class2"
✓ Available alternatives:
  1. None
  2. class "class")"));
        }

        SECTION("unmatched class with 2 variants") {
            auto matcher = MatcherDataclass("class") | MatcherDataclass("class").arguments({"arg1", "arg2"});
            CHECK_THAT(matcher.match(Parser::parse("class(1)"), result),
                       UnmatchedWithReason(
R"(Matching Alternative failed: all 2 variants of class "class" failed to match:
✖ (variant 1) Matching class "class" failed:
  ✖ Expected 0 positional arguments, but 1 was given
  ✓ Arguments specification:
    - standard arguments: empty
✖ (variant 2) Matching class "class" failed:
  ✖ Missing 1 required positional argument: "arg2"
  ✓ Arguments specification:
    - standard arguments:
      - arg1
      - arg2)"));
        }
    }
}