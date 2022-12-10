//
// Created by pkua on 10.12.22.
//

#include <catch2/catch.hpp>

#include "pyon/Matcher.h"
#include "pyon/Parser.h"

using namespace pyon;
using namespace pyon::matcher;


TEST_CASE("Matcher: Int") {
    Any result;

    SECTION("default") {
        CHECK_FALSE(MatcherInt{}.match(Parser::parse("True"), result));
        CHECK(MatcherInt{}.match(Parser::parse("7"), result));
        CHECK(result.as<long>() == 7);
    }

    SECTION("filters") {
        SECTION("positive") {
            auto matcher = MatcherInt{}.positive();
            CHECK_FALSE(matcher.match(Parser::parse("0"), result));
            CHECK(matcher.match(Parser::parse("7"), result));
        }

        SECTION("negative") {
            auto matcher = MatcherInt{}.negative();
            CHECK_FALSE(matcher.match(Parser::parse("0"), result));
            CHECK(matcher.match(Parser::parse("-7"), result));
        }

        SECTION("non-positive") {
            auto matcher = MatcherInt{}.nonPositive();
            CHECK_FALSE(matcher.match(Parser::parse("1"), result));
            CHECK(matcher.match(Parser::parse("0"), result));
            CHECK(matcher.match(Parser::parse("-1"), result));
        }

        SECTION("non-negative") {
            auto matcher = MatcherInt{}.nonNegative();
            CHECK_FALSE(matcher.match(Parser::parse("-1"), result));
            CHECK(matcher.match(Parser::parse("0"), result));
            CHECK(matcher.match(Parser::parse("1"), result));
        }

        SECTION("greater") {
            auto matcher = MatcherInt{}.greater(5);
            CHECK_FALSE(matcher.match(Parser::parse("5"), result));
            CHECK(matcher.match(Parser::parse("6"), result));
        }

        SECTION("greater equals") {
            auto matcher = MatcherInt{}.greaterEquals(5);
            CHECK_FALSE(matcher.match(Parser::parse("4"), result));
            CHECK(matcher.match(Parser::parse("5"), result));
        }

        SECTION("less") {
            auto matcher = MatcherInt{}.less(5);
            CHECK_FALSE(matcher.match(Parser::parse("5"), result));
            CHECK(matcher.match(Parser::parse("4"), result));
        }

        SECTION("less equals") {
            auto matcher = MatcherInt{}.lessEquals(5);
            CHECK_FALSE(matcher.match(Parser::parse("6"), result));
            CHECK(matcher.match(Parser::parse("5"), result));
        }

        SECTION("equals") {
            auto matcher1 = MatcherInt{}.equals(5);
            auto matcher2 = MatcherInt(5);
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse("4"), result));
            CHECK_FALSE(matcher.match(Parser::parse("6"), result));
            CHECK(matcher.match(Parser::parse("5"), result));
        }

        SECTION("any of") {
            auto matcher1 = MatcherInt{}.anyOf({1, 3, 5});
            auto matcher2 = MatcherInt{1, 3, 5};
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse("2"), result));
            CHECK(matcher.match(Parser::parse("1"), result));
            CHECK(matcher.match(Parser::parse("5"), result));
        }

        SECTION("in range") {
            auto matcher = MatcherInt{}.inRange(1, 3);
            CHECK_FALSE(matcher.match(Parser::parse("0"), result));
            CHECK_FALSE(matcher.match(Parser::parse("4"), result));
            CHECK(matcher.match(Parser::parse("1"), result));
            CHECK(matcher.match(Parser::parse("3"), result));
        }

        SECTION("custom") {
            auto matcher = MatcherInt{}.filter([](long i) { return i % 2 == 0; });
            CHECK_FALSE(matcher.match(Parser::parse("3"), result));
            CHECK(matcher.match(Parser::parse("4"), result));
        }

        SECTION("joined") {
            auto matcher = MatcherInt{}
                .greater(5)
                .filter([](long i) { return i % 2 == 0; });
            CHECK_FALSE(matcher.match(Parser::parse("4"), result));
            CHECK_FALSE(matcher.match(Parser::parse("7"), result));
            CHECK(matcher.match(Parser::parse("8"), result));
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
    CHECK_FALSE(MatcherFloat{}.match(Parser::parse("True"), result));
    CHECK(MatcherFloat{}.match(Parser::parse("7.5"), result));
    CHECK(result.as<double>() == 7.5);
}

TEST_CASE("Matcher: Boolean") {
    Any result;

    SECTION("default") {
        CHECK_FALSE(MatcherBoolean{}.match(Parser::parse("56"), result));
        CHECK(MatcherBoolean{}.match(Parser::parse("True"), result));
        CHECK(result.as<bool>());
        CHECK(MatcherBoolean{}.match(Parser::parse("False"), result));
        CHECK_FALSE(result.as<bool>());
    }

    SECTION("filters") {
        SECTION("true") {
            auto matcher1 = MatcherBoolean{true};
            auto matcher2 = MatcherBoolean{}.isTrue();
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse("False"), result));
            CHECK(matcher.match(Parser::parse("True"), result));
        }

        SECTION("false") {
            auto matcher1 = MatcherBoolean{false};
            auto matcher2 = MatcherBoolean{}.isFalse();
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse("True"), result));
            CHECK(matcher.match(Parser::parse("False"), result));
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

TEST_CASE("Matcher: None") {
    Any result;

    SECTION("default map") {
        MatcherNone matcher;
        CHECK_FALSE(matcher.match(Parser::parse("45"), result));
        CHECK(matcher.match(Parser::parse("None"), result));
        CHECK(result.isEmpty());
    }

    SECTION("custom map") {
        auto matcher = MatcherNone{}.mapTo([]() -> int { return 45; } );
        CHECK(matcher.match(Parser::parse("None"), result));
        CHECK(result.as<int>() == 45);
    }
}

TEST_CASE("Matcher: String") {
    Any result;

    SECTION("default") {
        CHECK_FALSE(MatcherString{}.match(Parser::parse("True"), result));
        CHECK(MatcherString{}.match(Parser::parse(R"("abc")"), result));
        CHECK(result.as<std::string>() == "abc");
    }

    SECTION("filters") {
        SECTION("equals") {
            auto matcher1 = MatcherString{}.equals("abc");
            auto matcher2 = MatcherString("abc");
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse(R"("def")"), result));
            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
        }

        SECTION("any of") {
            auto matcher1 = MatcherString{}.anyOf({"abc", "def", "ghi"});
            auto matcher2 = MatcherString{"abc", "def", "ghi"};
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse(R"("jkl")"), result));
            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
            CHECK(matcher.match(Parser::parse(R"("ghi")"), result));
        }

        SECTION("starts with") {
            auto matcher = MatcherString{}.startsWith("abc");
            CHECK_FALSE(matcher.match(Parser::parse(R"("123456")"), result));
            CHECK(matcher.match(Parser::parse(R"("abcdef")"), result));
        }

        SECTION("ends with") {
            auto matcher = MatcherString{}.endsWith("def");
            CHECK_FALSE(matcher.match(Parser::parse(R"("123456")"), result));
            CHECK(matcher.match(Parser::parse(R"("abcdef")"), result));
        }

        SECTION("contains") {
            auto matcher = MatcherString{}.contains("cd");
            CHECK_FALSE(matcher.match(Parser::parse(R"("123456")"), result));
            CHECK(matcher.match(Parser::parse(R"("abcdef")"), result));
        }

        SECTION("length") {
            auto matcher = MatcherString{}.length(3);
            CHECK_FALSE(matcher.match(Parser::parse(R"("abcd")"), result));
            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
        }

        SECTION("empty") {
            auto matcher = MatcherString{}.empty();
            CHECK_FALSE(matcher.match(Parser::parse(R"("abc")"), result));
            CHECK(matcher.match(Parser::parse(R"("")"), result));
        }

        SECTION("non-empty") {
            auto matcher = MatcherString{}.nonEmpty();
            CHECK_FALSE(matcher.match(Parser::parse(R"("")"), result));
            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
        }

        SECTION("contains only characters") {
            SECTION("given") {
                auto matcher = MatcherString{}.containsOnlyCharacters("rampckisol ");
                CHECK_FALSE(matcher.match(Parser::parse(R"("rampack is bad")"), result));
                CHECK(matcher.match(Parser::parse(R"("rampack is cool")"), result));
            }

            SECTION("predicate") {
                auto isFromPartialAlphabet = [](char c) { return c >= 'b' && c <= 'y'; };
                auto matcher = MatcherString{}.containsOnlyCharacters(isFromPartialAlphabet);
                CHECK_FALSE(matcher.match(Parser::parse(R"("abcxyz")"), result));
                CHECK(matcher.match(Parser::parse(R"("bcxy")"), result));
            }
        }

        SECTION("unique characters") {
            auto matcher = MatcherString{}.uniqueCharacters();
            CHECK_FALSE(matcher.match(Parser::parse(R"("abcdbe")"), result));
            CHECK(matcher.match(Parser::parse(R"("deacb")"), result));
        }

        SECTION("lowercase") {
            auto matcher = MatcherString{}.lowercase();
            CHECK_FALSE(matcher.match(Parser::parse(R"("AbC")"), result));
            CHECK(matcher.match(Parser::parse(R"("abc")"), result));
        }

        SECTION("uppercase") {
            auto matcher = MatcherString{}.uppercase();
            CHECK_FALSE(matcher.match(Parser::parse(R"("AbC")"), result));
            CHECK(matcher.match(Parser::parse(R"("ABC")"), result));
        }

        SECTION("numeric") {
            auto matcher = MatcherString{}.numeric();
            CHECK_FALSE(matcher.match(Parser::parse(R"("0x123")"), result));
            CHECK(matcher.match(Parser::parse(R"("123")"), result));
        }

        SECTION("alpha") {
            auto matcher = MatcherString{}.alpha();
            CHECK_FALSE(matcher.match(Parser::parse(R"("aBc123")"), result));
            CHECK(matcher.match(Parser::parse(R"("aBc")"), result));
        }

        SECTION("alphanumeric") {
            auto matcher = MatcherString{}.alphanumeric();
            CHECK_FALSE(matcher.match(Parser::parse(R"("aBc123@#")"), result));
            CHECK(matcher.match(Parser::parse(R"("aBc123")"), result));
        }

        SECTION("custom") {
            auto spaceFilter = [](const std::string &str) {
                return std::all_of(str.begin(), str.end(), [](char c) { return std::isspace(c); });
            };
            auto matcher = MatcherString{}.filter(spaceFilter);
            CHECK_FALSE(matcher.match(Parser::parse(R"("a b")"), result));
            CHECK(matcher.match(Parser::parse(R"("\t  \n\n")"), result));
        }

        SECTION("joined") {
            auto matcher = MatcherString{}
                .length(3)
                .lowercase()
                .uniqueCharacters()
                .containsOnlyCharacters("xyz");
            CHECK_FALSE(matcher.match(Parser::parse(R"("xy")"), result));
            CHECK_FALSE(matcher.match(Parser::parse(R"("xxz")"), result));
            CHECK(matcher.match(Parser::parse(R"("xzy")"), result));
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