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
}