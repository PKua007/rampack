//
// Created by Piotr Kubala on 23/01/2023.
//

#include "FileSnapshotWriterMatcher.h"
#include "FileSnapshotWriter.h"
#include "core/io/RamsnapWriter.h"
#include "core/io/WolframWriter.h"
#include "core/io/XYZWriter.h"


using namespace pyon::matcher;

namespace {
    auto filename = MatcherString{}.nonEmpty();


    MatcherDataclass create_ramsnap() {
        return MatcherDataclass("ramsnap")
            .arguments({{"filename", filename}})
            .mapTo([](const DataclassData &ramsnap) {
                auto filename = ramsnap["filename"].as<std::string>();
                auto writer = std::make_shared<RamsnapWriter>();
                FileSnapshotWriter fileWriter(filename, "RAMSNAP", std::move(writer));
                return fileWriter;
            });
    }

    MatcherDataclass create_wolfram() {
        auto style = MatcherDataclass("standard") | MatcherDataclass("affine_transform");

        return MatcherDataclass("wolfram")
            .arguments({{"filename", filename},
                        {"style", style, "standard"}})
            .variadicKeywordArguments(MatcherDictionary{}.valuesMatch(MatcherString{}))
            .mapTo([](const DataclassData &wolfram) {
                auto filename = wolfram["filename"].as<std::string>();
                auto style = wolfram["style"].as<WolframWriter::WolframStyle>();
                auto params = wolfram.getVariadicKeywordArguments().asStdMap<std::string>();
                auto writer = std::make_shared<WolframWriter>(style, params);
                FileSnapshotWriter fileWriter(filename, "Wolfram", std::move(writer));
                return fileWriter;
            });
    }

    MatcherDataclass create_xyz() {
        return MatcherDataclass("xyz")
            .arguments({{"filename", filename}})
            .mapTo([](const DataclassData &xyz) {
                auto filename = xyz["filename"].as<std::string>();
                auto writer = std::make_shared<XYZWriter>();
                FileSnapshotWriter fileWriter(filename, "XYZ", std::move(writer));
                return fileWriter;
            });
    }
}


pyon::matcher::MatcherAlternative FileSnapshotWriterMatcher::create() {
    return create_ramsnap() | create_wolfram() | create_xyz();
}
