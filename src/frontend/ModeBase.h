//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_MODEBASE_H
#define RAMPACK_MODEBASE_H

#include <vector>
#include <memory>
#include <fstream>

#include <cxxopts.hpp>

#include "IO.h"
#include "utils/Logger.h"


#define SHELL_SPECIAL_CHARACTERS_WARNING \
    "It is advisable to put the argument in single quotes `' '` to escape special shell characters `\"|()[]{}`"


class ModeBase {
private:
    std::ofstream auxOutStream;

protected:
    Logger &logger;
    IO io;

    void setVerbosityLevel(std::optional<std::string> verbosity, std::optional<std::string> auxOutput,
                           std::optional<std::string> auxVerbosity);
    Logger::LogType parseVerbosityLevel(const std::string &verbosityLevelName) const;
    cxxopts::ParseResult parseOptions(cxxopts::Options &options, int argc, char **argv) const;

public:
    explicit ModeBase(Logger &logger) : logger{logger}, io(logger) { }
    ~ModeBase() { this->logger.removeOutput(this->auxOutStream); }
};


#endif //RAMPACK_MODEBASE_H
