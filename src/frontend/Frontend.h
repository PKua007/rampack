//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_FRONTEND_H
#define RAMPACK_FRONTEND_H

#include <vector>
#include <core/ObservablesCollector.h>

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
    std::vector<Shape> arrangePacking(std::size_t numOfParticles, const std::array<double, 3> &boxDimensions,
                                      const std::string &arrangementString);
    void printAverageValues(const ObservablesCollector &collector);
    void storeAverageValues(const std::string &filename, const ObservablesCollector &collector, double temperature,
                            double pressure) const;

public:
    explicit Frontend(Logger &logger) : logger{logger} { }

    int casino(int argc, char **argv);
    int analyze(int argc, char **argv);

    int printGeneralHelp(const std::string &cmd);
};


#endif //RAMPACK_FRONTEND_H
