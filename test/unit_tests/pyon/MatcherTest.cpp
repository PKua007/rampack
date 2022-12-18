//
// Created by pkua on 10.12.22.
//

#include <catch2/catch.hpp>

#include "pyon/Matcher.h"
#include "pyon/Parser.h"

using namespace pyon;
using namespace pyon::ast;
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
    REQUIRE(MatcherFloat{}.match(Parser::parse("7.5"), result));
    CHECK(result.as<double>() == 7.5);
    REQUIRE(MatcherFloat{}.match(Parser::parse("7"), result));
    CHECK(result.as<double>() == 7);
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

TEST_CASE("Matcher: Array") {
    Any result;

    SECTION("default") {
        CHECK_FALSE(MatcherArray{}.match(Parser::parse("True"), result));
        CHECK(MatcherArray{}.match(Parser::parse("[1, 2, 3]"), result));
        auto array = result.as<ArrayData>();
        REQUIRE(array.size() == 3);
        CHECK(array[0].asNode<NodeInt>()->getValue() == 1);
        CHECK(array[1].asNode<NodeInt>()->getValue() == 2);
        CHECK(array[2].asNode<NodeInt>()->getValue() == 3);
    }

    SECTION("element matcher") {
        auto matcher1 = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<std::size_t>());
        auto matcher2 = MatcherArray(MatcherInt{}.mapTo<std::size_t>());
        auto matcher = GENERATE_COPY(matcher1, matcher2);
        CHECK_FALSE(matcher.match(Parser::parse("[True, False]"), result));
        CHECK(matcher.match(Parser::parse("[1, 2]"), result));
        auto array = result.as<ArrayData>();
        CHECK(array[0].as<std::size_t>() == 1);
        CHECK(array[1].as<std::size_t>() == 2);
    }

    SECTION("filters") {
        SECTION("custom filter") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>())
                    .filter([](const ArrayData &array) { return array.size() % 2 == 0; });
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3, 4]"), result));
        }

        SECTION("size") {
            auto matcher1 = MatcherArray(MatcherInt{}.mapTo<int>(), 3);
            auto matcher2 = MatcherArray(MatcherInt{}.mapTo<int>()).size(3);
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
        }

        SECTION("naked size constructor") {
            auto matcher1 = MatcherArray(std::size_t{3});
            auto matcher2 = MatcherArray(int{3});
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse("[1, True]"), result));
            CHECK(matcher.match(Parser::parse("[1, True, 1.2]"), result));
        }

        SECTION("size at least") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>()).sizeAtLeast(3);
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3, 4]"), result));
        }

        SECTION("size at most") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>()).sizeAtMost(3);
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2, 3, 4]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2]"), result));
        }

        SECTION("size in range") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>()).sizeInRange(2, 4);
            CHECK_FALSE(matcher.match(Parser::parse("[1]"), result));
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2, 3, 4, 5]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3, 4]"), result));
        }

        SECTION("empty") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>()).empty();
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.match(Parser::parse("[]"), result));
        }

        SECTION("non empty") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>()).nonEmpty();
            CHECK_FALSE(matcher.match(Parser::parse("[]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2]"), result));
        }

        SECTION("joined") {
            auto matcher = MatcherArray{}
                .elementsMatch(MatcherInt{}.positive().mapTo<int>())
                .sizeAtLeast(2)
                .sizeAtMost(4);
            CHECK_FALSE(matcher.match(Parser::parse("[1]"), result));
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2, 3, 4, 5]"), result));
            CHECK_FALSE(matcher.match(Parser::parse("[-1, 2, 3]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
        }
    }

    SECTION("maps") {
        SECTION("map to std::array") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>(), 3).mapToStdArray<int, 3>();
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(result.as<std::array<int, 3>>() == std::array<int, 3>{1, 2, 3});
        }

        SECTION("map to std::vector") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>()).mapToStdVector<int>();
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(result.as<std::vector<int>>() == std::vector<int>{1, 2, 3});
        }

        SECTION("map to Vector") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<double>(), 3).mapToVector<3>();
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(result.as<Vector<3>>() == Vector<3>{1, 2, 3});
        }

        SECTION("map to Matrix") {
            auto elementMatcher = MatcherInt{}.mapTo<double>();
            auto matcher = MatcherArray(MatcherArray(elementMatcher, 3), 2).mapToMatrix<2, 3>();
            CHECK(matcher.match(Parser::parse("[[1, 2, 3], [4, 5, 6]]"), result));
            CHECK(result.as<Matrix<2, 3>>() == Matrix<2, 3>{1, 2, 3,
                                                            4, 5, 6});
        }

        SECTION("custom map") {
            auto summedArray = [](const ArrayData &array) {
                auto plus = [](int sum, const Any &next) { return sum + next.as<int>(); };
                return std::accumulate(array.begin(), array.end(), 0, plus);
            };
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>()).mapTo(summedArray);
            CHECK(matcher.match(Parser::parse("[0, 2, 3]"), result));
            CHECK(result.as<int>() == 5);
        }
    }
}

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
        auto matcher1 = MatcherDictionary{}.valuesMatch(MatcherInt{}.mapTo<std::size_t>());
        auto matcher2 = MatcherDictionary(MatcherInt{}.mapTo<std::size_t>());
        auto matcher = GENERATE_COPY(matcher1, matcher2);
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
        }

        SECTION("has keys") {
            auto matcher = MatcherDictionary{}.hasKeys({"b", "a"});
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "c" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
        }

        SECTION("has only keys") {
            auto matcher = MatcherDictionary{}.hasOnlyKeys({"a", "b"});
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "c" : 2})"), result));
            CHECK_FALSE(matcher.match(Parser::parse(R"({"b" : 1, "a" : 2, "c" : 3})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1})"), result));
        }

        SECTION("has not keys") {
            auto matcher = MatcherDictionary{}.hasNotKeys({"c", "d"});
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "d" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
        }

        SECTION("keys match") {
            auto matcher = MatcherDictionary{}.keysMatch([](const std::string &key) { return key.length() == 1; });
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "bb" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
        }

        SECTION("empty") {
            auto matcher = MatcherDictionary{}.empty();
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse("{}"), result));
        }

        SECTION("size") {
            auto matcher = MatcherDictionary{}.size(2);
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1})"), result));
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
        }

        SECTION("sizeAtLeast") {
            auto matcher = MatcherDictionary{}.sizeAtLeast(2);
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
        }

        SECTION("sizeAtMost") {
            auto matcher = MatcherDictionary{}.sizeAtMost(2);
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1})"), result));
        }

        SECTION("sizeInRange") {
            auto matcher = MatcherDictionary{}.sizeInRange(2, 4);
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1})"), result));
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3, "d" : 4, "e" : 5})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3, "d" : 4})"), result));
        }

        SECTION("joined") {
            auto matcher = MatcherDictionary{}
                .size(3)
                .hasKeys({"a", "b"})
                .hasNotKeys({"c", "d"});
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2})"), result));
            CHECK_FALSE(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "c" : 3})"), result));
            CHECK(matcher.match(Parser::parse(R"({"a" : 1, "b" : 2, "e" : 5})"), result));
        }
    }

    SECTION("maps") {
        SECTION("map to std::map") {
            auto matcher = MatcherDictionary(MatcherInt{}.mapTo<int>()).mapToStdMap<int>();
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

TEST_CASE("Matcher: Dataclass") {
    Any result;

    SECTION("argument specification") {
        SECTION("empty") {
            auto matcher = MatcherDataclass("class");
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK_FALSE(matcher.match(Parser::parse("not_class()"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(3)"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(unwanted_argument=3)"), result));
            REQUIRE(matcher.match(Parser::parse("class()"), result));
            auto resultClass = result.as<DataclassData>();
            CHECK(resultClass.empty());
            CHECK(resultClass.positionalEmpty());
            CHECK(resultClass.getVariadicArguments().empty());
            CHECK(resultClass.getVariadicKeywordArguments().empty());
        }

        SECTION("arguments without matcher and default values") {
            auto matcher1 = MatcherDataclass("class", {"arg1", "arg2"});
            auto matcher2 = MatcherDataclass("class", {{"arg1"}, {"arg2"}});
            auto matcher3 = MatcherDataclass("class")
                    .arguments({{"arg1"}, {"arg2"}});
            auto matcher = GENERATE_COPY(matcher1, matcher2, matcher3);
            CHECK_FALSE(matcher.match(Parser::parse("class()"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(1)"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(1, 2, 3)"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(arg1=1)"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(arg1=1, arg3=3)"), result));

            auto input = GENERATE("class(1, 2)", "class(arg2=2, arg1=1)", "class(1, arg2=2)");
            DYNAMIC_SECTION("input: " << input) {
                REQUIRE(matcher.match(Parser::parse(input), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 2);
                REQUIRE(resultClass.positionalSize() == 2);
                CHECK(resultClass[0].asNode<NodeInt>()->getValue() == 1);
                CHECK(resultClass[1].asNode<NodeInt>()->getValue() == 2);
                CHECK(resultClass.getVariadicArguments().empty());
                CHECK(resultClass.getVariadicKeywordArguments().empty());
            }
        }

        SECTION("arguments with matcher") {
            auto matcher = MatcherDataclass("class", {{"arg1", MatcherInt{}}, {"arg2", MatcherInt{}}});
            CHECK_FALSE(matcher.match(Parser::parse("class(1, 2, 3)"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(arg1=1, arg3=3)"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(1, True)"), result));

            auto input = GENERATE("class(1, 2)", "class(arg2=2, arg1=1)", "class(1, arg2=2)");
            DYNAMIC_SECTION("input: " << input) {
                REQUIRE(matcher.match(Parser::parse(input), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 2);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 2);
            }
        }

        SECTION("arguments with matcher and default value") {
            auto matcher = MatcherDataclass("class", {{"arg1", MatcherInt{}},
                                                      {"arg2", MatcherInt{}, long{2}}});
            CHECK_FALSE(matcher.match(Parser::parse("class()"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(1, 2, 3)"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(1, True)"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(arg2=5)"), result));

            SECTION("defaulted") {
                auto input = GENERATE("class(1)", "class(arg1=1)");
                DYNAMIC_SECTION("input: " << input) {
                    REQUIRE(matcher.match(Parser::parse(input), result));
                    auto resultClass = result.as<DataclassData>();
                    REQUIRE(resultClass.size() == 2);
                    CHECK(resultClass[0].as<long>() == 1);
                    CHECK(resultClass[1].as<long>() == 2);
                }
            }

            SECTION("not defaulted") {
                auto input = GENERATE("class(1, 3)", "class(arg2=3, arg1=1)");
                DYNAMIC_SECTION("input: " << input) {
                    REQUIRE(matcher.match(Parser::parse(input), result));
                    auto resultClass = result.as<DataclassData>();
                    REQUIRE(resultClass.size() == 2);
                    CHECK(resultClass[0].as<long>() == 1);
                    CHECK(resultClass[1].as<long>() == 3);
                }
            }
        }

        SECTION("error: overriding positional with keyword") {
            auto matcher = MatcherDataclass("class", {{"arg1", MatcherInt{}}, {"arg2", MatcherInt{}}})
                .variadicKeywordArguments();
            CHECK_FALSE(matcher.match(Parser::parse("class(1, 2, arg1=3)"), result));
        }

        SECTION("variadic arguments") {
            auto matcher = MatcherDataclass("class")
                .arguments({{"arg1", MatcherInt{}}, {"arg2", MatcherInt{}, long{2}}})
                .variadicArguments(MatcherArray(MatcherInt{}));
            CHECK_FALSE(matcher.match(Parser::parse("class()"), result));

            SECTION("1 argument") {
                REQUIRE(matcher.match(Parser::parse("class(1)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 2);
                REQUIRE(resultClass.positionalSize() == 2);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 2);
                CHECK(resultClass.getVariadicArguments().empty());
                CHECK(resultClass.getVariadicKeywordArguments().empty());
            }

            SECTION("2 arguments") {
                REQUIRE(matcher.match(Parser::parse("class(1, 3)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 2);
                REQUIRE(resultClass.positionalSize() == 2);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 3);
                CHECK(resultClass.getVariadicArguments().empty());
                CHECK(resultClass.getVariadicKeywordArguments().empty());
            }

            SECTION("2 arguments + 1 variadic") {
                REQUIRE(matcher.match(Parser::parse("class(1, 3, 4)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 3);
                REQUIRE(resultClass.positionalSize() == 3);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 3);
                CHECK(resultClass[2].as<long>() == 4);
                CHECK(resultClass.getVariadicArguments().size() == 1);
                CHECK(resultClass.getVariadicArguments().at(0).as<long>() == 4);
                CHECK(resultClass.getVariadicKeywordArguments().empty());
            }
        }

        SECTION("variadic arguments with matcher") {
            auto matcher = MatcherDataclass("class")
                .variadicArguments(MatcherArray(MatcherInt{}).sizeAtLeast(1));
            CHECK_FALSE(matcher.match(Parser::parse("class()"), result));
            CHECK(matcher.match(Parser::parse("class(1)"), result));
            CHECK(matcher.match(Parser::parse("class(1, 2)"), result));
        }

        SECTION("keyword arguments") {
            auto matcher = MatcherDataclass("class")
                .arguments({{"arg1", MatcherInt{}}, {"arg2", MatcherInt{}, long{2}}})
                .variadicKeywordArguments(MatcherDictionary(MatcherInt{}));
            CHECK_FALSE(matcher.match(Parser::parse("class()"), result));
            CHECK_FALSE(matcher.match(Parser::parse("class(1, 2, 3)"), result));

            SECTION("1 argument") {
                REQUIRE(matcher.match(Parser::parse("class(1)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 2);
                REQUIRE(resultClass.positionalSize() == 2);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 2);
                CHECK(resultClass.getVariadicArguments().empty());
                CHECK(resultClass.getVariadicKeywordArguments().empty());
            }

            SECTION("2 arguments") {
                REQUIRE(matcher.match(Parser::parse("class(1, 3)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 2);
                REQUIRE(resultClass.positionalSize() == 2);
                CHECK(resultClass.getVariadicKeywordArguments().empty());
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 3);
                CHECK(resultClass.getVariadicArguments().empty());
                CHECK(resultClass.getVariadicKeywordArguments().empty());
            }

            SECTION("2 arguments + variadic keyword") {
                REQUIRE(matcher.match(Parser::parse("class(1, arg2=3, arg3=4, arg4=5)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 4);
                REQUIRE(resultClass.positionalSize() == 2);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 3);
                CHECK(resultClass.getVariadicArguments().empty());
                const auto &keywordArguments = resultClass.getVariadicKeywordArguments();
                REQUIRE(keywordArguments.size() == 2);
                CHECK(keywordArguments.at("arg3").as<long>() == 4);
                CHECK(keywordArguments.at("arg4").as<long>() == 5);
            }
        }

        SECTION("variadic keyword arguments with matcher") {
            auto matcher = MatcherDataclass("class")
                .variadicKeywordArguments(MatcherDictionary(MatcherInt{}).sizeAtLeast(1));
            CHECK_FALSE(matcher.match(Parser::parse("class()"), result));
            CHECK(matcher.match(Parser::parse("class(arg1=1)"), result));
            CHECK(matcher.match(Parser::parse("class(arg1=1, arg2=2)"), result));
        }

        SECTION("all types of arguments") {
            auto matcher = MatcherDataclass("class")
                .arguments({{"arg1", MatcherInt{}}, {"arg2", MatcherInt{}, long{2}}})
                .variadicArguments(MatcherArray(MatcherInt{}))
                .variadicKeywordArguments(MatcherDictionary(MatcherInt{}));
            CHECK_FALSE(matcher.match(Parser::parse("class()"), result));

            SECTION("1 argument") {
                REQUIRE(matcher.match(Parser::parse("class(1)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 2);
                REQUIRE(resultClass.positionalSize() == 2);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 2);
                CHECK(resultClass.getVariadicArguments().empty());
                CHECK(resultClass.getVariadicKeywordArguments().empty());
            }

            SECTION("2 arguments") {
                REQUIRE(matcher.match(Parser::parse("class(1, 3)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 2);
                REQUIRE(resultClass.positionalSize() == 2);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 3);
                CHECK(resultClass.getVariadicArguments().empty());
                CHECK(resultClass.getVariadicKeywordArguments().empty());
            }

            SECTION("2 + 1 variadic") {
                REQUIRE(matcher.match(Parser::parse("class(1, 3, 4)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 3);
                REQUIRE(resultClass.positionalSize() == 3);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 3);
                CHECK(resultClass.getVariadicArguments().size() == 1);
                CHECK(resultClass[2].as<long>() == 4);
                CHECK(resultClass.getVariadicKeywordArguments().empty());
            }

            SECTION("2 + 1 variadic keyword") {
                REQUIRE(matcher.match(Parser::parse("class(1, 3, arg3=4)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 3);
                REQUIRE(resultClass.positionalSize() == 2);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 3);
                CHECK(resultClass.getVariadicArguments().empty());
                const auto &keywordArguments = resultClass.getVariadicKeywordArguments();
                REQUIRE(keywordArguments.size() == 1);
                CHECK(keywordArguments.at("arg3").as<long>() == 4);
            }

            SECTION("2 + 1 variadic + 1 variadic keyword") {
                REQUIRE(matcher.match(Parser::parse("class(1, 3, 4, arg4=5)"), result));
                auto resultClass = result.as<DataclassData>();
                REQUIRE(resultClass.size() == 4);
                REQUIRE(resultClass.positionalSize() == 3);
                CHECK(resultClass[0].as<long>() == 1);
                CHECK(resultClass[1].as<long>() == 3);
                CHECK(resultClass.getVariadicArguments().size() == 1);
                CHECK(resultClass[2].as<long>() == 4);
                const auto &keywordArguments = resultClass.getVariadicKeywordArguments();
                REQUIRE(keywordArguments.size() == 1);
                CHECK(keywordArguments.at("arg4").as<long>() == 5);
            }
        }
    }

    SECTION("filter") {
        auto validateRange = [](const DataclassData &dataclass) {
            return dataclass["start"].as<long>() <= dataclass["end"].as<long>();
        };
        auto matcher = MatcherDataclass("range", {{"start", MatcherInt{}}, {"end", MatcherInt{}}})
            .filter(validateRange);
        CHECK_FALSE(matcher.match(Parser::parse("range(4, 2)"), result));
        CHECK(matcher.match(Parser::parse("range(2, 4)"), result));
    }

    SECTION("map to") {
        auto createRange = [](const DataclassData &dataclass) {
            auto start = dataclass["start"].as<long>();
            auto end = dataclass["end"].as<long>();
            std::vector<long> result(end - start + 1);
            std::iota(result.begin(), result.end(), start);
            return result;
        };
        auto matcher = MatcherDataclass("range", {{"start", MatcherInt{}}, {"end", MatcherInt{}}})
            .mapTo(createRange);
        REQUIRE(matcher.match(Parser::parse("range(2, 4)"), result));
        CHECK(result.as<std::vector<long>>() == std::vector<long>{2, 3, 4});
    }
}

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
}

TEST_CASE("Matcher: combined") {
    auto doConcatenate =
        [](const auto &arraylike, const std::string &beg, const std::string &end, const std::string &delim) {
            std::ostringstream result;
            result << beg;
            for (std::size_t i{}; i < arraylike.size(); i++) {
                result << arraylike[i].template as<std::string>();
                if (i != arraylike.size() - 1)
                    result << delim;
            }
            result << end;
            return result.str();
        };

    auto matcherInt = MatcherInt{}.mapTo([](long i) { return std::to_string(i); });
    auto matcherFloat = MatcherFloat{}.mapTo([](double d) { return std::to_string(d); });
    auto matcherString = MatcherString{};
    auto anyPrimitive = matcherInt | matcherFloat | matcherString;
    auto matcherArray = MatcherArray(anyPrimitive).mapTo([doConcatenate](const ArrayData &array) {
        return doConcatenate(array, "[", "]", ", ");
    });
    auto anyPrintable = anyPrimitive | matcherArray;
    auto kwargsDelimiter = MatcherDictionary(MatcherString{})
        .hasOnlyKeys({"delimiter"});
    auto concatenator = MatcherDataclass("concatenator")
        .variadicArguments(MatcherArray(anyPrintable))
        .variadicKeywordArguments(kwargsDelimiter)
        .mapTo([doConcatenate](const DataclassData &dataclass) {
            std::string delimiter = " ";
            if (!dataclass.getVariadicKeywordArguments().empty())
                delimiter = dataclass["delimiter"].as<std::string>();
            return doConcatenate(dataclass.getVariadicArguments(), "", "", delimiter);
        });

    Any result;
    REQUIRE(concatenator.match(Parser::parse(R"(concatenator(1, 1.2, "abc", [2, "e"], delimiter="-"))"), result));
    CHECK(result.as<std::string>() == "1-1.200000-abc-[2, e]");
    REQUIRE(concatenator.match(Parser::parse(R"(concatenator(1, 1.2, "abc", [2, "e"]))"), result));
    CHECK(result.as<std::string>() == "1 1.200000 abc [2, e]");
}