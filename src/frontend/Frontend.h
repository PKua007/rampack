//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_FRONTEND_H
#define RAMPACK_FRONTEND_H

#include <vector>
#include <memory>
#include <fstream>

#include "core/ObservablesCollector.h"
#include "utils/Logger.h"
#include "Parameters.h"
#include "core/Shape.h"
#include "core/BoundaryConditions.h"
#include "core/Interaction.h"
#include "core/lattice/OrthorhombicArrangingModel.h"
#include "core/Simulation.h"

/**
 * @brief Class responsible for the communication between the user and the simulation backend.
 */
class Frontend {
private:
    Logger &logger;
    std::ofstream auxOutStream;

    Parameters loadParameters(const std::string &inputFilename);
    void overwriteMoveStepSizes(const std::vector<std::unique_ptr<MoveSampler>> &moveSamplers,
                                const std::map<std::string, std::string> &packingAuxInfo) const;
    void appendMoveStepSizesToAuxInfo(const std::vector<std::unique_ptr<MoveSampler>> &moveSamplers,
                                      double scalingStepSize, std::map<std::string, std::string> &auxInfo) const;
    void setVerbosityLevel(std::optional<std::string> verbosity, std::optional<std::string> auxOutput,
                           std::optional<std::string> auxVerbosity);
    Logger::LogType parseVerbosityLevel(const std::string &verbosityLevelName) const;
    std::unique_ptr<SimulationRecorder> loadSimulationRecorder(const std::string &filename, bool &isContinuation) const;

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

    static std::string doubleToString(double d);
    static std::string formatMoveKey(const std::string &groupName, const std::string &moveName);
    static bool isStepSizeKey(const std::string &key);

public:
    explicit Frontend(Logger &logger) : logger{logger} { }
    ~Frontend() { this->logger.removeOutput(this->auxOutStream); }

    int casino(int argc, char **argv);
    int optimize_distance(int argc, char **argv);
    int preview(int argc, char **argv);
    int trajectory(int argc, char **argv);

    int printGeneralHelp(const std::string &cmd);
};


#endif //RAMPACK_FRONTEND_H
