//
// Created by Piotr Kubala on 12/03/2024.
//

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
        .valuesMatch(ShapeMatcher::createShapeData())
        .filter([](const DictionaryData &params) {
            return is_map_bijective(params.asStdMap<TextualShapeData>());
        })
        .describe("unique data for each species")
        .mapToStdMap<TextualShapeData>();

    return shapeSpeciesMap;
}