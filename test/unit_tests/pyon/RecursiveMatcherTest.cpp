//
// Created by Piotr Kubala on 01/06/2023.
//

#include <catch2/catch.hpp>

#include "matchers/UnmatchedWithReason.h"

#include "pyon/Matcher.h"
#include "pyon/Parser.h"

using namespace pyon;
using namespace pyon::ast;
using namespace pyon::matcher;


#include <iostream>

TEST_CASE("Matcher: RecursiveMatcher") {
    RecursiveMatcher recursiveNode;
    auto treeNodeArray = MatcherArray(recursiveNode, 2)
        .mapTo([](const ArrayData &data) -> std::string{
            auto left = data[0].as<std::string>();
            auto right = data[1].as<std::string>();
            return "[" + left + ", " + right + "]";
        });
    auto treeNodeInt = MatcherInt{}.mapTo([](long i) -> std::string { return std::to_string(i); });
    auto treeNodeNone = MatcherNone{}.mapTo([]() -> std::string { return "NULL"; });
    auto treeNode = treeNodeArray | treeNodeInt | treeNodeNone;
    recursiveNode.attach(treeNode);

    Any result;

    SECTION("not-matching") {
        CHECK_THAT(treeNode.match(Parser::parse("[1, [True, 2]]"), result),
                   UnmatchedWithReason(R"(Matching Array failed: Matching index 1 failed:
✖ Matching Array failed: Matching index 0 failed:
  ✖ Matching Alternative failed:
    ✖ Got incorrect node type: Boolean
    ✓ Available alternatives:
      1. Array[{recursion}]
      2. Integer
      3. None)"));
    }

    SECTION("matching") {
        REQUIRE(treeNode.match(Parser::parse("[[1, None], [[None, [2, 3]], None]]"), result));
        CHECK(result.as<std::string>() == "[[1, NULL], [[NULL, [2, 3]], NULL]]");
    }

    SECTION("outline") {
        CHECK(treeNode.outline(4) == R"(    Alternative:
    1. Array:
       - with elements: {recursion on (Array[{recursion}] | Integer | None)}
       - with size = 2
    2. Integer
    3. None)");
    }

    SECTION("synopsis") {
        CHECK(treeNode.synopsis() == "Array[{recursion}] | Integer | None");
    }
}