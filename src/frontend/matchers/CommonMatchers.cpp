//
// Created by Piotr Kubala on 12/03/2024.
//

#include <iterator>

#include "CommonMatchers.h"
#include "ShapeMatcher.h"


using namespace pyon::matcher;


const MatcherString &CommonMatchers::createSymbol() {
    static auto symbol = MatcherString{}
        .nonEmpty()
        .filter([](const std::string &str) {
            if (!std::all_of(str.begin(), str.end(), [](char c) { return c == '_' || std::isalnum(c); }))
                return false;
            if (std::isdigit(str.front()))
                return false;
            return true;
        })
        .describe("valid symbol name (only letters, numbers and underscore; doesn't start with a number)");

    return symbol;
}

const MatcherDictionary &CommonMatchers::createShapeSpeciesMap() {
    static auto shapeSpeciesMap = MatcherDictionary{}
        .keysMatch(CommonMatchers::createSymbol())
        .valuesMatch(CommonMatchers::createShapeData())
        .filter([](const DictionaryData &params) {
            return is_map_bijective(params.asStdMap<TextualShapeData>());
        })
        .describe("unique data for each species")
        .mapToStdMap<TextualShapeData>();

    return shapeSpeciesMap;
}

const pyon::matcher::MatcherAlternative &CommonMatchers::createShapeParamValue() {
    static std::optional<MatcherAlternative> shapeParamValue = std::nullopt;

    if (!shapeParamValue.has_value()) {
        auto longMatcher = MatcherInt{}.mapTo([](long i) -> std::string {
            return std::to_string(i);
        });
        auto doubleMatcher = MatcherFloat{}.mapTo([](double d) -> std::string {
            std::ostringstream out;
            out << std::setprecision(std::numeric_limits<double>::max_digits10) << d;
            return out.str();
        });
        auto stringMatcher = MatcherString{}
            .filter([](const std::string &str) {
                return std::none_of(str.begin(), str.end(), [](char c) { return std::isspace(c) || c == '"'; });
            })
            .describe("not containing whitespace or quotation marks (\")");
        auto vectorMatcher = MatcherArray(MatcherFloat{}, 3)
            .mapTo([](const ArrayData &arrayData) -> std::string {
                auto vec = arrayData.asVector<3>();
                std::ostringstream out;
                out << std::setprecision(std::numeric_limits<double>::max_digits10);
                std::copy(vec.begin(), vec.end(), std::ostream_iterator<double>(out, ","));
                return out.str();
            });

        shapeParamValue = longMatcher | doubleMatcher | stringMatcher | vectorMatcher;
    }

    return *shapeParamValue;
}

const pyon::matcher::MatcherDictionary &CommonMatchers::createShapeData() {
    static auto shapeData = MatcherDictionary{}
        .keysMatch(CommonMatchers::createSymbol())
        .valuesMatch(CommonMatchers::createShapeParamValue())
        .mapToStdMap<std::string>();

    return shapeData;
}
