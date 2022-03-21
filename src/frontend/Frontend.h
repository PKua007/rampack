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
#include "core/BoundaryConditions.h"
#include "core/Interaction.h"
#include "core/arranging_models/OrthorhombicArrangingModel.h"
#include "core/Simulation.h"

/**
 * @brief Class responsible for the communication between the user and the simulation backend.
 */
class Frontend {
private:
    Logger &logger;

    Parameters loadParameters(const std::string &inputFilename);
    void setVerbosityLevel(const std::string &verbosityLevelName) const;

    void performIntegration(Simulation &simulation, const Parameters::IntegrationParameters &runParams,
                            const ShapeTraits &shapeTraits, size_t cycleOffset, bool isContinuation);

    void performOverlapRelaxation(Simulation &simulation, const std::string &shapeName, const std::string &shapeAttr,
                                  const Parameters::OverlapRelaxationParameters &runParams,
                                  std::shared_ptr<ShapeTraits> shapeTraits, size_t cycleOffset, bool isContinuation);

    void printPerformanceInfo(const Simulation &simulation);
    void printAverageValues(const ObservablesCollector &collector);

    void storeAverageValues(const std::string &filename, const ObservablesCollector &collector, double temperature,
                            double pressure) const;
    void storePacking(const Simulation &simulation, const std::string &packingFilename);
    void storeWolframVisualization(const Simulation &simulation, const ShapeTraits &shapeTraits,
                                   const std::string &wolframFilename) const;
    void storeSnapshots(const ObservablesCollector &observablesCollector, bool isContinuation,
                        const std::string &observableSnapshotFilename) const;

    void printMoveStatistics(const Simulation &simulation) const;

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
