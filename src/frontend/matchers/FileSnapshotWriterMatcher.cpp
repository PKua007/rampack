//
// Created by Piotr Kubala on 23/01/2023.
//

#include "FileSnapshotWriterMatcher.h"
#include "CommonMatchers.h"
#include "ShapeMatcher.h"
#include "frontend/FileSnapshotWriter.h"
#include "core/io/RamsnapWriter.h"
#include "core/io/WolframWriter.h"
#include "core/io/XYZWriter.h"


using namespace pyon::matcher;

namespace {
    auto filename = MatcherString{}.nonEmpty();


    MatcherDataclass create_ramsnap() {
        return MatcherDataclass("ramsnap")
            .arguments({{"filename", filename}})
            .mapTo([](const DataclassData &ramsnap) -> std::shared_ptr<FileSnapshotWriter> {
                auto filename = ramsnap["filename"].as<std::string>();
                return std::make_shared<RamsnapFileSnapshotWriter>(filename);
            });
    }

    MatcherDataclass create_wolfram() {
        auto standard = MatcherDataclass("standard")
            .mapTo([](const DataclassData &) { return WolframWriter::WolframStyle::STANDARD; });
        auto affineTransform = MatcherDataclass("affine_transform")
            .mapTo([](const DataclassData &) { return WolframWriter::WolframStyle::AFFINE_TRANSFORM; });
        auto style = standard | affineTransform;

        return MatcherDataclass("wolfram")
            .arguments({{"filename", filename},
                        {"style", style, "standard"}})
            .variadicKeywordArguments(MatcherDictionary{}.valuesMatch(MatcherString{}))
            .mapTo([](const DataclassData &wolfram) -> std::shared_ptr<FileSnapshotWriter> {
                auto filename = wolfram["filename"].as<std::string>();
                auto style = wolfram["style"].as<WolframWriter::WolframStyle>();
                auto params = wolfram.getVariadicKeywordArguments().asStdMap<std::string>();
                return std::make_shared<WolframFileSnapshotWriter>(std::move(filename), style, std::move(params));
            });
    }

    MatcherDataclass create_xyz() {
        auto speciesMap = MatcherDictionary{}
            .keysMatch(CommonMatchers::symbol)
            .valuesMatch(ShapeMatcher::createShapeData())
            .mapToStdMap<TextualShapeData>();

        return MatcherDataclass("xyz")
            .arguments({{"filename", filename},
                        {"species_map", speciesMap, "{}"}})
            .mapTo([](const DataclassData &xyz) -> std::shared_ptr<FileSnapshotWriter>  {
                auto filename = xyz["filename"].as<std::string>();
                auto speciesMap = xyz["species_map"].as<std::map<std::string, TextualShapeData>>();
                return std::make_shared<XYZFileSnapshotWriter>(std::move(filename), std::move(speciesMap));
            });
    }
}


pyon::matcher::MatcherAlternative FileSnapshotWriterMatcher::create() {
    return create_ramsnap() | create_wolfram() | create_xyz();
}

std::shared_ptr<FileSnapshotWriter> FileSnapshotWriterMatcher::match(const std::string &expression) {
    auto writerMatcher = FileSnapshotWriterMatcher::create();
    auto writerAST = pyon::Parser::parse(expression);
    Any writer;
    auto matchReport = writerMatcher.match(writerAST, writer);
    if (!matchReport)
        throw pyon::matcher::MatchException(matchReport.getReason());

    return writer.as<std::shared_ptr<FileSnapshotWriter>>();
}
