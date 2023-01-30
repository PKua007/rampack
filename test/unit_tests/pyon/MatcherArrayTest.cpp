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
        auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<std::size_t>());
        CHECK_FALSE(matcher.match(Parser::parse("[True, False]"), result));
        CHECK(matcher.match(Parser::parse("[1, 2]"), result));
        auto array = result.as<ArrayData>();
        CHECK(array[0].as<std::size_t>() == 1);
        CHECK(array[1].as<std::size_t>() == 2);
    }

    SECTION("synopsis") {
        CHECK(MatcherArray{}.synopsis() == "Array[any expression]");

        auto arrayArrayFloat = MatcherArray{}.elementsMatch(MatcherArray{}.elementsMatch(MatcherFloat{}));
        CHECK(arrayArrayFloat.synopsis() == "Array[Array[Float]]");
    }

    SECTION("outline layout") {
        CHECK(MatcherArray{}.outline(4) == "    Array");
        CHECK(MatcherArray{}.elementsMatch(MatcherInt{}).outline(4) == "    Array, with elements: Integer");
        CHECK(MatcherArray{}.elementsMatch(MatcherInt{}.positive()).outline(4) == "    Array, with elements: Integer, > 0");

        CHECK(MatcherArray{}.elementsMatch(MatcherInt{}.positive().less(5)).outline(4) == R"(    Array:
    - with elements: Integer:
      - > 0
      - < 5)");

        CHECK(MatcherArray(MatcherInt{}.positive(), 3).outline(4) == R"(    Array:
    - with elements: Integer, > 0
    - with size = 3)");
    }

    SECTION("filters") {
        SECTION("custom filter") {
            auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<int>())
                    .filter([](const ArrayData &array) { return array.size() % 2 == 0; });
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3, 4]"), result));

            CHECK(matcher.outline(4) == R"(    Array:
    - with elements: Integer
    - <undefined filter>)");

            matcher.describe("with even size");
            CHECK(matcher.outline(4) == R"(    Array:
    - with elements: Integer
    - with even size)");
        }

        SECTION("size") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>(), 3);
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(matcher.outline(4) == R"(    Array:
    - with elements: Integer
    - with size = 3)");
        }

        SECTION("naked size constructor") {
            auto matcher1 = MatcherArray(std::size_t{3});
            auto matcher2 = MatcherArray(int{3});
            auto matcher = GENERATE_COPY(matcher1, matcher2);
            CHECK_FALSE(matcher.match(Parser::parse("[1, True]"), result));
            CHECK(matcher.match(Parser::parse("[1, True, 1.2]"), result));
            CHECK(matcher.outline(4) == R"(    Array, with size = 3)");
        }

        SECTION("size at least") {
            auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<int>()).sizeAtLeast(3);
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3, 4]"), result));
            CHECK(matcher.outline(4) == R"(    Array:
    - with elements: Integer
    - with size >= 3)");
        }

        SECTION("size at most") {
            auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<int>()).sizeAtMost(3);
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2, 3, 4]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.outline(4) == R"(    Array:
    - with elements: Integer
    - with size <= 3)");
        }

        SECTION("size in range") {
            auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<int>()).sizeInRange(2, 4);
            CHECK_FALSE(matcher.match(Parser::parse("[1]"), result));
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2, 3, 4, 5]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2, 3, 4]"), result));
            CHECK(matcher.outline(4) == R"(    Array:
    - with elements: Integer
    - with size in range [2, 4])");
        }

        SECTION("empty") {
            auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<int>()).empty();
            CHECK_FALSE(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.match(Parser::parse("[]"), result));
            CHECK(matcher.outline(4) == R"(    Array:
    - with elements: Integer
    - empty)");
        }

        SECTION("non empty") {
            auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<int>()).nonEmpty();
            CHECK_FALSE(matcher.match(Parser::parse("[]"), result));
            CHECK(matcher.match(Parser::parse("[1, 2]"), result));
            CHECK(matcher.outline(4) == R"(    Array:
    - with elements: Integer
    - non-empty)");
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
            CHECK(matcher.outline(4) == R"(    Array:
    - with elements: Integer, > 0
    - with size >= 2
    - with size <= 4)");
        }
    }

    SECTION("error reporting") {
        SECTION("node type") {
            CHECK_THAT(MatcherArray{}.match(Parser::parse("True"), result),
                       UnmatchedWithReason(R"(Matching Array failed:
✖ Got incorrect node type: Boolean
✓ Expected format: Array)"));
        }

        SECTION("value unmatched") {
            auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.positive().less(5));
            CHECK_THAT(matcher.match(Parser::parse("[3, 6]"), result),
                       UnmatchedWithReason(R"(Matching Array failed: Matching index 1 failed:
✖ Matching Integer failed:
  ✖ Condition not satisfied: < 5
  ✓ Expected format: Integer:
    - > 0
    - < 5)"));
        }

        SECTION("filter unmatched") {
            auto matcher = MatcherArray{}
                .sizeAtLeast(2)
                .sizeAtMost(4);
            CHECK_THAT(matcher.match(Parser::parse("[1, 2, 3, 4, 5]"), result),
                       UnmatchedWithReason(R"(Matching Array failed:
✖ Condition not satisfied: with size <= 4
✓ Expected format: Array:
  - with size >= 2
  - with size <= 4)"));
        }
    }

    SECTION("maps") {
        SECTION("map to std::array") {
            auto matcher = MatcherArray(MatcherInt{}.mapTo<int>(), 3).mapToStdArray<int, 3>();
            CHECK(matcher.match(Parser::parse("[1, 2, 3]"), result));
            CHECK(result.as<std::array<int, 3>>() == std::array<int, 3>{1, 2, 3});
        }

        SECTION("map to std::vector") {
            auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<int>()).mapToStdVector<int>();
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
            auto matcher = MatcherArray{}.elementsMatch(MatcherInt{}.mapTo<int>()).mapTo(summedArray);
            CHECK(matcher.match(Parser::parse("[0, 2, 3]"), result));
            CHECK(result.as<int>() == 5);
        }
    }
}