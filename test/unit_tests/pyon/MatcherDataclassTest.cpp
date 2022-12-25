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
                                                      {"arg2", MatcherInt{}, "2"}});
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
                    .arguments({{"arg1", MatcherInt{}}, {"arg2", MatcherInt{}, "2"}})
                    .variadicArguments(MatcherArray{}.elementsMatch(MatcherInt{}));
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
                    .variadicArguments(MatcherArray{}.elementsMatch(MatcherInt{}).sizeAtLeast(1));
            CHECK_FALSE(matcher.match(Parser::parse("class()"), result));
            CHECK(matcher.match(Parser::parse("class(1)"), result));
            CHECK(matcher.match(Parser::parse("class(1, 2)"), result));
        }

        SECTION("keyword arguments") {
            auto matcher = MatcherDataclass("class")
                    .arguments({{"arg1", MatcherInt{}}, {"arg2", MatcherInt{}, "2"}})
                    .variadicKeywordArguments(MatcherDictionary{}.valuesMatch(MatcherInt{}));
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
                    .variadicKeywordArguments(MatcherDictionary{}.valuesMatch(MatcherInt{}).sizeAtLeast(1));
            CHECK_FALSE(matcher.match(Parser::parse("class()"), result));
            CHECK(matcher.match(Parser::parse("class(arg1=1)"), result));
            CHECK(matcher.match(Parser::parse("class(arg1=1, arg2=2)"), result));
        }

        SECTION("all types of arguments") {
            auto matcher = MatcherDataclass("class")
                    .arguments({{"arg1", MatcherInt{}}, {"arg2", MatcherInt{}, "2"}})
                    .variadicArguments(MatcherArray{}.elementsMatch(MatcherInt{}))
                    .variadicKeywordArguments(MatcherDictionary{}.valuesMatch(MatcherInt{}));
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

    SECTION("outline") {
        SECTION("empty") {
            CHECK(MatcherDataclass("clazz").outline(4) == "    clazz class:\n    - arguments: empty");
        }

        SECTION("standard arguments") {
            auto matcher = MatcherDataclass("clazz")
                    .arguments({{"arg1"},
                                {"arg2", MatcherInt{}},
                                {"arg3", MatcherFloat{}.positive().less(6), "5"}});
            CHECK(matcher.outline(4) == R"(    clazz class:
    - arguments:
      - arg1: any expression
      - arg2: Integer
      - arg3 (=5): Float:
        - > 0
        - < 6)");
        }

        SECTION("*args") {
            SECTION("short") {
                auto matcher = MatcherDataclass("clazz")
                        .variadicArguments(MatcherArray{}.sizeAtLeast(5));
                CHECK(matcher.outline(4) == R"(    clazz class:
    - arguments: empty
    - *args: Array, with size >= 5)");
            }

            SECTION("long") {
                auto matcher = MatcherDataclass("clazz")
                        .variadicArguments(MatcherArray{}.sizeAtLeast(5).sizeAtMost(7));
                CHECK(matcher.outline(4) == R"(    clazz class:
    - arguments: empty
    - *args: Array:
      - with size >= 5
      - with size <= 7)");
            }
        }

        SECTION("**kwargs") {
            SECTION("short") {
                auto matcher = MatcherDataclass("clazz")
                        .variadicKeywordArguments(MatcherDictionary{}.sizeAtLeast(5));
                CHECK(matcher.outline(4) == R"(    clazz class:
    - arguments: empty
    - **kwargs: Dictionary, with size >= 5)");
            }

            SECTION("long") {
                auto matcher = MatcherDataclass("clazz")
                        .variadicKeywordArguments(MatcherDictionary{}.sizeAtLeast(5).sizeAtMost(7));
                CHECK(matcher.outline(4) == R"(    clazz class:
    - arguments: empty
    - **kwargs: Dictionary:
      - with size >= 5
      - with size <= 7)");
            }
        }

        SECTION("all at once") {
            auto matcher = MatcherDataclass("clazz")
                    .arguments({{"arg1", MatcherInt{}}})
                    .variadicArguments(MatcherArray{}.sizeAtLeast(5).sizeAtMost(7))
                    .variadicKeywordArguments(MatcherDictionary{}.nonEmpty());
            CHECK(matcher.outline(4) == R"(    clazz class:
    - arguments:
      - arg1: Integer
    - *args: Array:
      - with size >= 5
      - with size <= 7
    - **kwargs: Dictionary, non-empty)");
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

        CHECK(matcher.outline(4) == R"(    range class:
    - arguments:
      - start: Integer
      - end: Integer
    - <undefined filter>)");

        matcher.describe("start <= end");
        CHECK(matcher.outline(4) == R"(    range class:
    - arguments:
      - start: Integer
      - end: Integer
    - start <= end)");
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