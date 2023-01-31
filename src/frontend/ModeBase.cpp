//
// Created by Piotr Kubala on 12/12/2020.
//

#include <filesystem>
#include <set>

#include <cxxopts.hpp>

#include "ModeBase.h"
#include "core/Packing.h"
#include "utils/Utils.h"
#include "utils/Exceptions.h"


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
        throw ValidationException("Unknown verbosity level: " + verbosityLevelName);
}

cxxopts::ParseResult ModeBase::parseOptions(cxxopts::Options &options, int argc, char **argv) const {
    cxxopts::ParseResult parsedOptions;
    try {
        parsedOptions = options.parse(argc, argv);
    } catch (const cxxopts::OptionParseException &ex) {
        throw ValidationException(std::string("An error occurred while parsing command line options:\n") + ex.what());
    }
    return parsedOptions;
}
