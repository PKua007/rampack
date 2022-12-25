//
// Created by pkua on 10.12.22.
//

// Silence not useful maybe uninitialized warning in GCC 12.2 on Apple Silicon in the Release build (when inlining is
// performed) appearing in some but no all test cases with MatcherInt and MatcherFloat (weird)
// <functional> has to be manually included before Catch2
#ifndef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    #include <functional>
    #pragma GCC diagnostic pop
#endif

#include <catch2/catch.hpp>

#include "matchers/UnmatchedWithReason.h"

#include "pyon/Matcher.h"
#include "pyon/Parser.h"

using namespace pyon;
using namespace pyon::ast;
using namespace pyon::matcher;


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
    auto matcherArray = MatcherArray{}.elementsMatch(anyPrimitive).mapTo([doConcatenate](const ArrayData &array) {
        return doConcatenate(array, "[", "]", ", ");
    });
    auto anyPrintable = anyPrimitive | matcherArray;
    auto kwargsDelimiter = MatcherDictionary{}.valuesMatch(MatcherString{})
        .hasOnlyKeys({"delimiter"});
    auto concatenator = MatcherDataclass("concatenator")
        .variadicArguments(MatcherArray{}.elementsMatch(anyPrintable))
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

    CHECK(concatenator.outline(0) == R"(concatenator class:
- arguments: empty
- *args: Array:
  - with elements: Alternative:
    1. Integer
    2. Float
    3. String
    4. Array:
       - with elements: Alternative:
         1. Integer
         2. Float
         3. String
- **kwargs: Dictionary:
  - with values: String
  - allowed keys: "delimiter")");
}