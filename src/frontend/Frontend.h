//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_FRONTEND_H
#define RAMPACK_FRONTEND_H

#include <memory>
#include <vector>

#include "utils/Logger.h"
#include "Parameters.h"
#include "core/Shape.h"

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

    std::vector<std::unique_ptr<Shape>> arrangePacking(std::size_t numOfParticles,
                                                       const std::array<double, 3> &boxDimensions,
                                                       const std::string &arrangementString);
};


#endif //RAMPACK_FRONTEND_H
