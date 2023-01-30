//
// Created by Piotr Kubala on 23/01/2023.
//

#include "FileShapePrinterMatcher.h"
#include "frontend/FileShapePrinter.h"


using namespace pyon::matcher;

namespace {
    auto filename = MatcherString{}.nonEmpty();

    MatcherDataclass create_wolfram(const ShapeTraits &traits) {
        return MatcherDataclass("wolfram")
            .arguments({{"filename", filename}})
            .variadicKeywordArguments(MatcherDictionary{}.valuesMatch(MatcherString{}))
            .mapTo([&traits](const DataclassData &wolfram) {
                auto filename = wolfram["filename"].as<std::string>();
                auto params = wolfram.getVariadicKeywordArguments().asStdMap<std::string>();
                auto printer = traits.getPrinter("wolfram", params);
                FileShapePrinter filePrinter(filename, "Wolfram", printer);
                return filePrinter;
            });
    }

    MatcherDataclass create_obj(const ShapeTraits &traits) {
        return MatcherDataclass("obj")
            .arguments({{"filename", filename}})
            .variadicKeywordArguments(MatcherDictionary{}.valuesMatch(MatcherString{}))
            .mapTo([&traits](const DataclassData &obj) {
                auto filename = obj["filename"].as<std::string>();
                auto params = obj.getVariadicKeywordArguments().asStdMap<std::string>();
                auto printer = traits.getPrinter("obj", params);
                FileShapePrinter filePrinter(filename, "Wavefront OBJ", printer);
                return filePrinter;
            });
    }
}


pyon::matcher::MatcherAlternative FileShapePrinterMatcher::create(const ShapeTraits &traits) {
    return create_wolfram(traits) | create_obj(traits);
}

FileShapePrinter FileShapePrinterMatcher::match(const std::string &expression, const ShapeTraits &traits) {
    auto printerMatcher = FileShapePrinterMatcher::create(traits);
    auto printerAST = pyon::Parser::parse(expression);
    Any printer;
    auto matchReport = printerMatcher.match(printerAST, printer);
    if (!matchReport)
        throw InputError(matchReport.getReason());

    return printer.as<FileShapePrinter>();
}
