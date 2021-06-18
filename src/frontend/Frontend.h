//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_FRONTEND_H
#define RAMPACK_FRONTEND_H

#include <vector>
#include <memory>

#include "core/ObservablesCollector.h"
#include "utils/Logger.h"
#include "Parameters.h"
#include "core/Shape.h"
#include "core/VolumeScaler.h"
#include "core/BoundaryConditions.h"
#include "core/Interaction.h"

/**
 * @brief Class responsible for the communication between the user and the simulation backend.
 */
class Frontend {
private:
    Logger &logger;

    Parameters loadParameters(const std::string &inputFilename, const std::vector<std::string> &overridenParams);
    void setVerbosityLevel(const std::string &verbosityLevelName) const;
    void setOverridenParamsAsAdditionalText(std::vector<std::string> overridenParams) const;
    std::unique_ptr<Packing> arrangePacking(std::size_t numOfParticles, const std::array<double, 3> &boxDimensions,
                                            const std::string &arrangementString,
                                            std::unique_ptr<BoundaryConditions> bc, const Interaction &interaction,
                                            std::size_t moveThreads, std::size_t scalingThreads);
    void printAverageValues(const ObservablesCollector &collector);
    void storeAverageValues(const std::string &filename, const ObservablesCollector &collector, double temperature,
                            double pressure) const;
    [[nodiscard]] std::unique_ptr<VolumeScaler> createVolumeScaler(std::string scalingType) const;

public:
    explicit Frontend(Logger &logger) : logger{logger} { }

    int casino(int argc, char **argv);
    int analyze(int argc, char **argv);
    int optimize_distance(int argc, char **argv);
    int preview(int argc, char **argv);

    int printGeneralHelp(const std::string &cmd);

};


#endif //RAMPACK_FRONTEND_H
