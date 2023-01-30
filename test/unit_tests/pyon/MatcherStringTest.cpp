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


TEST_CASE("Matcher: String") {
    Any result;

    SECTION("default") {
        CHECK_THAT(MatcherString{}.match(Parser::parse("True"), result),
                   UnmatchedWithReason(R"(Matching String failed:
✖ Got incorrect node type: Boolean
✓ Expected format: String)"));

        CHECK(MatcherString{}.match(Parser::parse(R"("abc")"), result));
        CHECK(result.as<std::string>() == "abc");
        CHECK(MatcherString{}.outline(4) == "    String");
    }

    SECTION("synopsis") {
        CHECK(MatcherString{}.synopsis() == "String");
    }

    SECTION("filters") {
        SECTION("equals") {
            auto matcher1 = MatcherString{}.equals("abc");
            auto matcher2 = MatcherString("abc");
            auto matcher = GENERATE_COPY(matcher1, matcher2);

            CHECK_THAT(matcher.match(Parser::parse(R"("def")"), result),
                       UnmatchedWithReason(R"(Matching String failed:
✖ Condition not satisfied: = "abc"
✓ Expected format: String, = "abc")"));

            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
            CHECK(matcher.outline(4) == R"(    String, = "abc")");
        }

        SECTION("any of") {
            auto matcher1 = MatcherString{}.anyOf({"abc", "def", "ghi"});
            auto matcher2 = MatcherString{"abc", "def", "ghi"};
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse(R"("jkl")"), result));
            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
            CHECK(matcher.match(Parser::parse(R"("ghi")"), result));
            CHECK(matcher.outline(4) == R"(    String, one of: "abc", "def", "ghi")");
        }

        SECTION("starts with") {
            auto matcher = MatcherString{}.startsWith("abc");
            CHECK_FALSE(matcher.match(Parser::parse(R"("123456")"), result));
            CHECK(matcher.match(Parser::parse(R"("abcdef")"), result));
            CHECK(matcher.outline(4) == R"(    String, starting with "abc")");
        }

        SECTION("ends with") {
            auto matcher = MatcherString{}.endsWith("def");
            CHECK_FALSE(matcher.match(Parser::parse(R"("123456")"), result));
            CHECK(matcher.match(Parser::parse(R"("abcdef")"), result));
            CHECK(matcher.outline(4) == R"(    String, ending with "def")");
        }

        SECTION("contains") {
            auto matcher = MatcherString{}.contains("cd");
            CHECK_FALSE(matcher.match(Parser::parse(R"("123456")"), result));
            CHECK(matcher.match(Parser::parse(R"("abcdef")"), result));
            CHECK(matcher.outline(4) == R"(    String, containing "cd")");
        }

        SECTION("length") {
            auto matcher = MatcherString{}.length(3);
            CHECK_FALSE(matcher.match(Parser::parse(R"("abcd")"), result));
            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
            CHECK(matcher.outline(4) == R"(    String, with length = 3)");
        }

        SECTION("empty") {
            auto matcher = MatcherString{}.empty();
            CHECK_FALSE(matcher.match(Parser::parse(R"("abc")"), result));
            CHECK(matcher.match(Parser::parse(R"("")"), result));
            CHECK(matcher.outline(4) == R"(    String, empty)");
        }

        SECTION("non-empty") {
            auto matcher = MatcherString{}.nonEmpty();
            CHECK_FALSE(matcher.match(Parser::parse(R"("")"), result));
            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
            CHECK(matcher.outline(4) == R"(    String, non-empty)");
        }

        SECTION("contains only characters") {
            SECTION("given") {
                auto matcher = MatcherString{}.containsOnlyCharacters("rampckisol ");
                CHECK_FALSE(matcher.match(Parser::parse(R"("rampack is bad")"), result));
                CHECK(matcher.match(Parser::parse(R"("rampack is cool")"), result));
                CHECK(matcher.outline(4) == R"(    String, with only characters: "rampckisol ")");
            }

            SECTION("predicate") {
                auto isFromPartialAlphabet = [](char c) { return c >= 'b' && c <= 'y'; };
                auto matcher = MatcherString{}.containsOnlyCharacters(isFromPartialAlphabet);
                CHECK_FALSE(matcher.match(Parser::parse(R"("abcxyz")"), result));
                CHECK(matcher.match(Parser::parse(R"("bcxy")"), result));
                CHECK(matcher.outline(4) == R"(    String, with only characters: <undefined predicate>)");
            }
        }

        SECTION("unique characters") {
            auto matcher = MatcherString{}.uniqueCharacters();
            CHECK_FALSE(matcher.match(Parser::parse(R"("abcdbe")"), result));
            CHECK(matcher.match(Parser::parse(R"("deacb")"), result));
            CHECK(matcher.outline(4) == R"(    String, with unique characters)");
        }

        SECTION("lowercase") {
            auto matcher = MatcherString{}.lowercase();
            CHECK_FALSE(matcher.match(Parser::parse(R"("AbC")"), result));
            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
            CHECK(matcher.outline(4) == R"(    String, lowercase)");
        }

        SECTION("uppercase") {
            auto matcher = MatcherString{}.uppercase();
            CHECK_FALSE(matcher.match(Parser::parse(R"("AbC")"), result));
            CHECK(matcher.match(Parser::parse(R"("ABC")"), result));
            CHECK(matcher.outline(4) == R"(    String, uppercase)");
        }

        SECTION("numeric") {
            auto matcher = MatcherString{}.numeric();
            CHECK_FALSE(matcher.match(Parser::parse(R"("0x123")"), result));
            CHECK(matcher.match(Parser::parse(R"("123")"), result));
            CHECK(matcher.outline(4) == R"(    String, with only numbers)");
        }

        SECTION("alpha") {
            auto matcher = MatcherString{}.alpha();
            CHECK_FALSE(matcher.match(Parser::parse(R"("aBc123")"), result));
            CHECK(matcher.match(Parser::parse(R"("aBc")"), result));
            CHECK(matcher.outline(4) == R"(    String, with only letters)");
        }

        SECTION("alphanumeric") {
            auto matcher = MatcherString{}.alphanumeric();
            CHECK_FALSE(matcher.match(Parser::parse(R"("aBc123@#")"), result));
            CHECK(matcher.match(Parser::parse(R"("aBc123")"), result));
            CHECK(matcher.outline(4) == R"(    String, with only numbers and letters)");
        }

        SECTION("custom") {
            auto spaceFilter = [](const std::string &str) {
                return std::all_of(str.begin(), str.end(), [](char c) { return std::isspace(c); });
            };
            auto matcher = MatcherString{}.filter(spaceFilter);
            CHECK_FALSE(matcher.match(Parser::parse(R"("a b")"), result));
            CHECK(matcher.match(Parser::parse(R"("\t  \n\n")"), result));
            CHECK(matcher.outline(4) == R"(    String, <undefined filter>)");
            matcher.describe("with only white characters");
            CHECK(matcher.outline(4) == R"(    String, with only white characters)");
        }

        SECTION("joined") {
            auto matcher = MatcherString{}
                    .length(3)
                    .lowercase()
                    .uniqueCharacters()
                    .containsOnlyCharacters("xyz");
            CHECK_FALSE(matcher.match(Parser::parse(R"("xy")"), result));

            CHECK_THAT(matcher.match(Parser::parse(R"("xxz")"), result),
                       UnmatchedWithReason(R"(Matching String failed:
✖ Condition not satisfied: with unique characters
✓ Expected format: String:
  - with length = 3
  - lowercase
  - with unique characters
  - with only characters: "xyz")"));

            CHECK(matcher.match(Parser::parse(R"("xzy")"), result));

            CHECK(matcher.outline(4) == R"(    String:
    - with length = 3
    - lowercase
    - with unique characters
    - with only characters: "xyz")");
        }
    }

    SECTION("map to") {
        auto matcher = MatcherString{}.mapTo<std::runtime_error>();
        CHECK(matcher.match(Parser::parse(R"("abc")"), result));
        CHECK(result.as<std::runtime_error>().what() == std::string("abc"));
    }

    SECTION("custom map") {
        auto matcher = MatcherString{}.mapTo([](const std::string &str) { return std::stoi(str); });
        CHECK(matcher.match(Parser::parse(R"("5")"), result));
        CHECK(result.as<int>() == 5);
    }
}