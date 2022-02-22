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
#include "core/TriclinicBoxScaler.h"
#include "core/BoundaryConditions.h"
#include "core/Interaction.h"
#include "core/arranging_models/OrthorhombicArrangingModel.h"

/**
 * @brief Class responsible for the communication between the user and the simulation backend.
 */
class Frontend {
private:
    Logger &logger;

    Parameters loadParameters(const std::string &inputFilename);
    void setVerbosityLevel(const std::string &verbosityLevelName) const;
    void printAverageValues(const ObservablesCollector &collector);
    void storeAverageValues(const std::string &filename, const ObservablesCollector &collector, double temperature,
                            double pressure) const;
    [[nodiscard]] std::unique_ptr<TriclinicBoxScaler> createTriclinicBoxScaler(const std::string &scalingType) const;
    [[nodiscard]] std::unique_ptr<VolumeScaler> createVolumeScaler(std::string scalingType) const;
    [[nodiscard]] std::array<double, 3> parseDimensions(const std::string &initialDimensions) const;
    std::string doubleToString(double d);

public:
    explicit Frontend(Logger &logger) : logger{logger} { }

    int casino(int argc, char **argv);
    int optimize_distance(int argc, char **argv);
    int preview(int argc, char **argv);

    int printGeneralHelp(const std::string &cmd);
};


#endif //RAMPACK_FRONTEND_H
