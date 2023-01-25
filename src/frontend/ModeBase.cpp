//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cstdlib>
#include <limits>
#include <iterator>
#include <filesystem>
#include <set>

#include <cxxopts.hpp>

#include "ModeBase.h"
#include "RampackParameters.h"
#include "core/Simulation.h"
#include "core/Packing.h"
#include "matchers/RampackMatcher.h"
#include "legacy/IniParametersFactory.h"
#include "utils/Fold.h"
#include "utils/Utils.h"
#include "utils/Assertions.h"


void ModeBase::setVerbosityLevel(std::optional<std::string> verbosity, std::optional<std::string> auxOutput,
                                 std::optional<std::string> auxVerbosity)
{
    if (verbosity.has_value())
        this->logger.setVerbosityLevel(this->parseVerbosityLevel(*verbosity));
    else if (auxOutput.has_value())
        this->logger.setVerbosityLevel(Logger::WARN);
    else
        this->logger.setVerbosityLevel(Logger::INFO);

    if (!auxOutput.has_value())
        return;

    this->auxOutStream.open(*auxOutput);
    ValidateOpenedDesc(this->auxOutStream, *auxOutput, "to log messages");

    this->logger.warn() << "Logging output to '" << *auxOutput << "'" << std::endl;

    this->logger.addOutput(this->auxOutStream);

    if (auxVerbosity.has_value())
        this->logger.setVerbosityLevel(this->parseVerbosityLevel(*auxVerbosity), this->auxOutStream);
    else
        this->logger.setVerbosityLevel(Logger::INFO, this->auxOutStream);
}

Logger::LogType ModeBase::parseVerbosityLevel(const std::string &verbosityLevelName) const {
    if (verbosityLevelName == "error")
        return Logger::ERROR;
    else if (verbosityLevelName == "warn")
        return Logger::WARN;
    else if (verbosityLevelName == "info")
        return Logger::INFO;
    else if (verbosityLevelName == "verbose")
        return Logger::VERBOSE;
    else if (verbosityLevelName == "debug")
        return Logger::DEBUG;
    else
        die("Unknown verbosity level: " + verbosityLevelName, this->logger);

    return Logger::ERROR;
}

void ModeBase::combineEnvironment(Simulation::Environment &env, const Run &run)
{
    auto environmentGetter = [](const auto &run) {
        return run.environment;
    };
    auto runEnv = std::visit(environmentGetter, run);
    env.combine(runEnv);
}

void ModeBase::createWalls(Packing &packing, const std::array<bool, 3> &walls) {
    for (std::size_t i{}; i < 3; i++)
        packing.toggleWall(i, walls[i]);
}