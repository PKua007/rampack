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


TEST_CASE("Matcher: Dictionary") {
    Any result;

    SECTION("default") {
        CHECK_FALSE(MatcherDictionary{}.match(Parser::parse("[1, 2, 3]"), result));

        REQUIRE(MatcherDictionary{}.match(Parser::parse(R"({"b" : 2, "a" : 1, "c" : 3})"), result));
        auto dict = result.as<DictionaryData>();
        REQUIRE(dict.size() == 3);
        CHECK(dict["a"].asNode<NodeInt>()->getValue() == 1);
        CHECK(dict["b"].asNode<NodeInt>()->getValue() == 2);
        CHECK(dict["c"].asNode<NodeInt>()->getValue() == 3);
    }

    SECTION("element matcher") {
        auto matcher = MatcherDictionary{}.valuesMatch(MatcherInt{}.mapTo<std::size_t>());
        CHECK_FALSE(matcher.match(Parser::parse(R"({"b" : 2, "a" : "not int", "c" : 3})"), result));

        REQUIRE(matcher.match(Parser::parse(R"({"b" : 2, "a" : 1, "c" : 3})"), result));
        auto dict = result.as<DictionaryData>();
        REQUIRE(dict.size() == 3);
        CHECK(dict["a"].as<std::size_t>() == 1);
        CHECK(dict["b"].as<std::size_t>() == 2);
        CHECK(dict["c"].as<std::size_t>() == 3);
    }

    SECTION("key-wise element matcher") {
        auto matcher = MatcherDictionary{}
                .valuesMatch(MatcherInt{})
                .valueAtKeyMatches("a", MatcherFloat{})
                .valueAtKeyMatches([](const std::string &key) { return key.length() > 3; }, MatcherBoolean{}.isTrue());
        CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1.2, "b" : "not int"})"), result));
        CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1.2, "longkey" : False})"), result));

        REQUIRE(matcher.match(Parser::parse(R"({"a" : 1.2, "b" : 1, "longkey" : True})"), result));
        auto dict = result.as<DictionaryData>();
        REQUIRE(dict.size() == 3);
        CHECK(dict["a"].as<double>() == 1.2);
        CHECK(dict["b"].as<long>() == 1);
        CHECK(dict["longkey"].as<bool>() == true);
    }

    SECTION("synopsis") {
        CHECK(MatcherDictionary{}.synopsis() == "Dictionary[String -> any expression]");
        CHECK(MatcherDictionary{}.valuesMatch(MatcherFloat{}).synopsis() == "Dictionary[String -> Float]");
        CHECK(MatcherDictionary{}.valueAtKeyMatches("key", MatcherNone{}).synopsis()
              == "Dictionary[String -> various]");
    }

    SECTION("outline layout") {
        CHECK(MatcherDictionary{}.outline(4) == "    Dictionary");
        CHECK(MatcherDictionary{}.valuesMatch(MatcherInt{}).outline(4) == "    Dictionary, with values: Integer");
        CHECK(MatcherDictionary{}.valuesMatch(MatcherInt{}.positive()).outline(4)
              == "    Dictionary, with values: Integer, > 0");

        CHECK(MatcherDictionary{}.valuesMatch(MatcherInt{}.positive().less(5)).outline(4) == R"(    Dictionary:
    - with values: Integer:
      - > 0
      - < 5)");

        CHECK(MatcherDictionary().valueAtKeyMatches("abc", MatcherInt{}).outline(4) == R"(    Dictionary:
    - with values at keys:
      - "abc" : Integer)");

        auto matcher = MatcherDictionary{}
                .valuesMatch(MatcherInt{})
                .valueAtKeyMatches("a", MatcherFloat{})
                .valueAtKeyMatches([](const std::string &key) { return key.length() > 3; }, MatcherBoolean{}.isTrue());

        CHECK(matcher.outline(4) == R"(    Dictionary:
    - with values at keys:
      - "a"                   : Float
      - <undefined predicate> : Boolean equal True
      - other keys            : Integer)");

        matcher.describeKey("keys with length > 3");

        CHECK(matcher.outline(4) == R"(    Dictionary:
    - with values at keys:
      - "a"                  : Float
      - keys with length > 3 : Boolean equal True
      - other keys           : Integer)");
    }

    SECTION("error reporting") {
        SECTION("node type") {
            CHECK_THAT(MatcherDictionary{}.match(Parser::parse("True"), result),
                       UnmatchedWithReason(R"(Matching Dictionary failed:
✖ Got incorrect node type: Boolean
✓ Expected format: Dictionary[String -> any expression])"));
        }

        SECTION("default value unmatched") {
            auto matcher = MatcherDictionary{}
                .valuesMatch(MatcherInt{}.positive().less(5))
                .valueAtKeyMatches("key1", MatcherString{}.alpha().nonEmpty());

            CHECK_THAT(matcher.match(Parser::parse(R"({"key1": "123"})"), result),
                       UnmatchedWithReason(R"(Matching Dictionary failed: Matching key "key1" failed:
✖ Matching String failed:
  ✖ Condition not satisfied: with only letters)"));

            CHECK_THAT(matcher.match(Parser::parse(R"({"key2": -1})"), result),
                       UnmatchedWithReason(R"(Matching Dictionary failed: Matching key "key2" failed:
✖ Matching Integer failed:
  ✖ Condition not satisfied: > 0)"));
        }

        SECTION("filter unmatched") {
            auto matcher = MatcherDictionary{}
                .sizeAtLeast(1)
                .sizeAtMost(2);
            CHECK_THAT(matcher.match(Parser::parse(R"({"key1": 1, "key2": 2, "key3": 3})"), result),
                       UnmatchedWithReason(R"(Matching Dictionary failed:
✖ Condition not satisfied: with size <= 2)"));
        }
    }

    SECTION("filters") {
        SECTION("custom filter") {
            auto keysAreAlphanumeric = [](const DictionaryData &dict) {
                for (const auto &[key, value] : dict)
                    if (!std::all_of(key.begin(), key.end(), [](char c) { return std::isalpha(c); }))
                        return false;
                return true;
            };
            auto matcher = MatcherDictionary{}
                    .filter(keysAreAlphanumeric);
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b0" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, <undefined filter>)");
            matcher.describe("with alphanumeric keys");
            CHECK(matcher.outline(4) == R"(    Dictionary, with alphanumeric keys)");
        }

        SECTION("has keys") {
            auto matcher = MatcherDictionary{}.hasKeys({"b", "a"});
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "c" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, mandatory keys: "b", "a")");
        }

        SECTION("has only keys") {
            auto matcher = MatcherDictionary{}.hasOnlyKeys({"a", "b"});
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "c" : 2})"), result));
            CHECK_FALSE(matcher.match(Parser::parse(R"({"b" : 1, "a" : 2, "c" : 3})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, allowed keys: "a", "b")");
        }

        SECTION("has not keys") {
            auto matcher = MatcherDictionary{}.hasNotKeys({"c", "d"});
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "d" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, forbidden keys: "c", "d")");
        }

        SECTION("keys match") {
            auto matcher = MatcherDictionary{}.keysMatch([](const std::string &key) { return key.length() == 1; });
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "bb" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, keys match <undefined predicate>)");
            matcher.describe("1-character keys");
            CHECK(matcher.outline(4) == R"(    Dictionary, 1-character keys)");
        }

        SECTION("empty") {
            auto matcher = MatcherDictionary{}.empty();
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse("{}"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, empty)");
        }

        SECTION("non-empty") {
            auto matcher = MatcherDictionary{}.nonEmpty();
            CHECK_FALSE(matcher.match(Parser::parse("{}"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, non-empty)");
        }

        SECTION("size") {
            auto matcher = MatcherDictionary{}.size(2);
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1})"), result));
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, with size = 2)");
        }

        SECTION("sizeAtLeast") {
            auto matcher = MatcherDictionary{}.sizeAtLeast(2);
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, with size >= 2)");
        }

        SECTION("sizeAtMost") {
            auto matcher = MatcherDictionary{}.sizeAtMost(2);
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, with size <= 2)");
        }

        SECTION("sizeInRange") {
            auto matcher = MatcherDictionary{}.sizeInRange(2, 4);
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1})"), result));
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3, "d" : 4, "e" : 5})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3, "d" : 4})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary, with size in range [2, 4])");
        }

        SECTION("joined") {
            auto matcher = MatcherDictionary{}
                    .size(3)
                    .hasKeys({"a", "b"})
                    .hasNotKeys({"c", "d"});
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "e" : 5})"), result));
            CHECK(matcher.outline(4) == R"(    Dictionary:
    - with size = 3
    - mandatory keys: "a", "b"
    - forbidden keys: "c", "d")");
        }
    }

    SECTION("maps") {
        SECTION("map to std::map") {
            auto matcher = MatcherDictionary{}.valuesMatch(MatcherInt{}.mapTo<int>()).mapToStdMap<int>();
            REQUIRE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            std::map<std::string, int> expected{{"a", 1}, {"b", 2}, {"c", 3}};
            CHECK(result.as<std::map<std::string, int>>() == expected);
        }

        SECTION("custom map") {
            auto concatenatedKeys = [](const DictionaryData &dict) {
                auto concatenate = [](const std::string &concat, const std::pair<std::string, Any> &next) {
                    return concat + next.first;
                };
                return std::accumulate(dict.begin(), dict.end(), std::string{}, concatenate);
            };
            auto matcher = MatcherDictionary{}.mapTo(concatenatedKeys);
            REQUIRE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            CHECK(result.as<std::string>() == "abc");
        }
    }
}