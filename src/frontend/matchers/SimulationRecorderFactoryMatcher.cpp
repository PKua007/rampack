//
// Created by Piotr Kubala on 23/01/2023.
//

#include "SimulationRecorderFactoryMatcher.h"
#include "frontend/SimulationRecorderFactory.h"

using namespace pyon::matcher;


namespace {
    auto filename = MatcherString{}.nonEmpty();


    MatcherDataclass create_ramtrj() {
        return MatcherDataclass("ramtrj")
            .arguments({{"filename", filename}})
            .mapTo([](const DataclassData &ramtrj) -> std::shared_ptr<SimulationRecorderFactory> {
                auto filename = ramtrj["filename"].as<std::string>();
                return std::make_shared<RamtrjRecorderFactory>(filename);
            });
    }

    MatcherDataclass create_xyz() {
        return MatcherDataclass("xyz")
            .arguments({{"filename", filename}})
            .mapTo([](const DataclassData &xyz) -> std::shared_ptr<SimulationRecorderFactory> {
                auto filename = xyz["filename"].as<std::string>();
                return std::make_shared<XYZRecorderFactory>(filename);
            });
    }
}


pyon::matcher::MatcherAlternative SimulationRecorderFactoryMatcher::create() {
    return create_ramtrj() | create_xyz();
}

std::shared_ptr<SimulationRecorderFactory>
SimulationRecorderFactoryMatcher::match(const std::string &expression) {
    auto factoryMatcher = SimulationRecorderFactoryMatcher::create();
    auto factoryAST = pyon::Parser::parse(expression);
    Any factory;
    auto matchReport = factoryMatcher.match(factoryAST, factory);
    if (!matchReport)
        throw InputError(matchReport.getReason());

    return factory.as<std::shared_ptr<SimulationRecorderFactory>>();
}
