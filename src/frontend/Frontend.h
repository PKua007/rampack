//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_FRONTEND_H
#define RAMPACK_FRONTEND_H

#include "utils/Logger.h"
#include "Parameters.h"

/**
 * @brief Class responsible for the communication between the user and the simulation backend.
 */
class Frontend {
private:
    Logger &logger;

    Parameters loadParameters(const std::string &inputFilename, const std::vector<std::string> &overridenParams);
    void setVerbosityLevel(const std::string &verbosityLevelName) const;
    void setOverridenParamsAsAdditionalText(std::vector<std::string> overridenParams) const;

public:
    explicit Frontend(Logger &logger) : logger{logger} { }

    int casino(int argc, char **argv);
    int analyze(int argc, char **argv);

    int printGeneralHelp(const std::string &cmd);
};


#endif //RAMPACK_FRONTEND_H
