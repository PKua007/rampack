//
// Created by Codex on 28/06/2026.
//

#include "RangeMatcher.h"
#include "utils/ParseUtils.h"
#include "utils/Utils.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <optional>
#include <set>
#include <utility>

using namespace pyon::matcher;


namespace {
    using RangeStrings = std::vector<std::pair<std::string, std::string>>;
    using Ranges = std::vector<std::pair<std::size_t, std::size_t>>;


    std::vector<std::string> splitRangeStringTokens(const std::string &rangeString) {
        auto tokens = explode(rangeString, ',');
        std::transform(tokens.begin(), tokens.end(), tokens.begin(), [](std::string token) {
            return trim(token);
        });
        return tokens;
    }

    bool consistsOfDigits(const std::string &token) {
        return !token.empty() && std::all_of(token.begin(), token.end(), [](unsigned char ch) {
            return std::isdigit(ch);
        });
    }

    bool isNonEmptyRangeString(const std::string &rangeString) {
        auto trimmedRangeString = rangeString;
        return !trim(trimmedRangeString).empty();
    }

    std::optional<std::pair<std::string, std::string>> parseRangeStringToken(const std::string &token) {
        auto separatorPos = token.find('-');
        if (separatorPos == std::string::npos) {
            if (!consistsOfDigits(token))
                return std::nullopt;
            return std::make_pair(token, "");
        }

        if (token.find('-', separatorPos + 1) != std::string::npos)
            return std::nullopt;
        if (!consistsOfDigits(token.substr(0, separatorPos)))
            return std::nullopt;
        if (!consistsOfDigits(token.substr(separatorPos + 1)))
            return std::nullopt;

        return std::make_pair(token.substr(0, separatorPos), token.substr(separatorPos + 1));
    }

    std::optional<RangeStrings> parseRangeStrings(const std::string &rangeString) {
        RangeStrings rangeStrings;
        for (const auto &token : splitRangeStringTokens(rangeString)) {
            auto rangeStringToken = parseRangeStringToken(token);
            if (!rangeStringToken.has_value())
                return std::nullopt;
            rangeStrings.push_back(std::move(*rangeStringToken));
        }

        if (rangeStrings.empty())
            return std::nullopt;
        return rangeStrings;
    }

    std::optional<Ranges> parseRanges(const std::string &rangeString) {
        auto rangeStrings = parseRangeStrings(rangeString);
        if (!rangeStrings.has_value())
            return std::nullopt;

        Ranges ranges;
        ranges.reserve(rangeStrings->size());

        try {
            for (const auto &[startString, endString] : *rangeStrings) {
                auto start = ParseUtils::parseIntegral<std::size_t>(startString);
                auto end = endString.empty() ? start : ParseUtils::parseIntegral<std::size_t>(endString);
                ranges.emplace_back(start, end);
            }
        } catch (const std::out_of_range&) {
            return std::nullopt;
        } catch (const std::invalid_argument&) {
            AssertThrow("unreachable: syntax validation should have caught it earlier");
        }

        return ranges;
    }

    bool areRangesAscending(const Ranges &ranges) {
        return std::all_of(ranges.begin(), ranges.end(), [](const auto &range) {
            return range.first <= range.second;
        });
    }

    std::optional<bool> hasAscendingRanges(const std::string &rangeString) {
        auto ranges = parseRanges(rangeString);
        if (!ranges.has_value())
            return std::nullopt;

        return areRangesAscending(*ranges);
    }

    std::optional<std::vector<std::size_t>> expandRanges(const std::string &rangeString) {
        auto ranges = parseRanges(rangeString);
        if (!ranges.has_value())
            return std::nullopt;
        if (!areRangesAscending(*ranges))
            return std::nullopt;

        std::vector<std::size_t> indices;

        for (const auto &[start, end] : *ranges) {
            Expects(start <= end);
            for (std::size_t index = start; index <= end; index++) {
                indices.push_back(index);
                if (index == std::numeric_limits<std::size_t>::max())
                    break;
            }
        }

        return indices;
    }

    bool areIndicesUnique(const std::vector<std::size_t> &indices) {
        std::set<std::size_t> uniqueIndices;
        for (auto index : indices) {
            if (!uniqueIndices.insert(index).second)
                return false;
        }
        return true;
    }

    std::vector<std::size_t> normalizeIndices(std::vector<std::size_t> indices) {
        std::sort(indices.begin(), indices.end());
        return indices;
    }

    std::optional<bool> hasUniqueRangeStringIndices(const std::string &rangeString) {
        auto indices = expandRanges(rangeString);
        if (!indices.has_value())
            return std::nullopt;

        return areIndicesUnique(*indices);
    }

    std::vector<std::size_t> parseRangeStringUnchecked(const std::string &rangeString) {
        auto indices = expandRanges(rangeString);
        Expects(indices.has_value());
        return normalizeIndices(std::move(*indices));
    }
}

MatcherAlternative RangeMatcher::create() {
    const auto rangeStringMatcher = MatcherString{}
        .filter(isNonEmptyRangeString)
        .describe("non-empty index range")
        .filter([](const std::string &rangeString) {
            return parseRangeStrings(rangeString).has_value();
        })
        .describe(R"(comma-separated non-negative indices or inclusive ranges, for example "1,3,10-20")")
        .filter([](const std::string &rangeString) {
            return parseRanges(rangeString).has_value();
        })
        .describe("with indices in the range [0, " + std::to_string(std::numeric_limits<std::size_t>::max()) + "]")
        .filter([](const std::string &rangeString) {
            return hasAscendingRanges(rangeString).value_or(false);
        })
        .describe("with ascending ranges")
        .filter([](const std::string &rangeString) {
            return hasUniqueRangeStringIndices(rangeString).value_or(false);
        })
        .describe("with unique indices")
        .mapTo([](const std::string &rangeString) {
            return parseRangeStringUnchecked(rangeString);
        });

    const auto rangeArrayMatcher = MatcherArray{}
        .elementsMatch(MatcherInt{}.nonNegative().mapTo<std::size_t>())
        .nonEmpty()
        .filter([](const ArrayData &arrayData) {
            return areIndicesUnique(arrayData.asStdVector<std::size_t>());
        })
        .describe("with unique indices")
        .mapTo([](const ArrayData &arrayData) {
            return normalizeIndices(arrayData.asStdVector<std::size_t>());
        });

    return rangeStringMatcher | rangeArrayMatcher;
}
