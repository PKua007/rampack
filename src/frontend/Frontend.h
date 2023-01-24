//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_FRONTEND_H
#define RAMPACK_FRONTEND_H

#include <vector>
#include <memory>
#include <fstream>

#include "PackingLoader.h"
#include "RampackParameters.h"
#include "FileShapePrinter.h"

#include "core/Shape.h"
#include "core/BoundaryConditions.h"
#include "core/Interaction.h"
#include "core/Simulation.h"
#include "core/io/RamtrjPlayer.h"

#include "utils/Version.h"
#include "utils/Logger.h"


/**
 * @brief Class responsible for the communication between the user and the simulation backend.
 */
class Frontend {
private:
    Logger &logger;
    std::ofstream auxOutStream;

    std::unique_ptr<Packing> recreatePacking(PackingLoader &loader, const BaseParameters &params,
                                             const ShapeTraits &traits, std::size_t maxThreads);

    void overwriteMoveStepSizes(Simulation::Environment &env,
                                const std::map<std::string, std::string> &packingAuxInfo) const;

    Simulation::Environment recreateEnvironment(const RampackParameters &params, const PackingLoader &loader) const;
    Simulation::Environment recreateRawEnvironment(const RampackParameters &params, std::size_t startRunIndex) const;

    void setVerbosityLevel(std::optional<std::string> verbosity, std::optional<std::string> auxOutput,
                           std::optional<std::string> auxVerbosity);

    Logger::LogType parseVerbosityLevel(const std::string &verbosityLevelName) const;
    std::unique_ptr<RamtrjPlayer> loadRamtrjPlayer(std::string &trajectoryFilename, size_t numMolecules, bool autoFix_);
    void createWalls(Packing &packing, const std::array<bool, 3> &walls);
    void attachSnapshotOut(ObservablesCollector &collector, const std::string& filename, bool isContinuation) const;

    void verifyDynamicParameter(const DynamicParameter &dynamicParameter, const std::string &parameterName,
                                const IntegrationRun &run, std::size_t cycleOffset) const;

    void performIntegration(Simulation &simulation, Simulation::Environment &env, const IntegrationRun &run,
                            const ShapeTraits &shapeTraits, std::size_t cycleOffset, bool isContinuation);

    void performOverlapRelaxation(Simulation &simulation, Simulation::Environment &env, const OverlapRelaxationRun &run,
                                  std::shared_ptr<ShapeTraits> shapeTraits, std::size_t cycleOffset,
                                  bool isContinuation);

    void printPerformanceInfo(const Simulation &simulation);
    void printAverageValues(const ObservablesCollector &collector);

    void storeAverageValues(const std::string &filename, const ObservablesCollector &collector, double temperature,
                            double pressure) const;

    void storeSnapshots(const ObservablesCollector &observablesCollector, bool isContinuation,
                        const std::string &observableSnapshotFilename) const;

    void storeBulkObservables(const ObservablesCollector &observablesCollector,
                              std::string bulkObservableFilenamePattern) const;

    void printMoveStatistics(const Simulation &simulation) const;
    void printInteractionInfo(const Interaction &interaction);
    void printGeometryInfo(const ShapeGeometry &geometry);
    static std::string formatMoveKey(const std::string &groupName, const std::string &moveName);
    static bool isStepSizeKey(const std::string &key);
    static void combineEnvironment(Simulation::Environment &env, const Run &run);
    RampackParameters dispatchParams(const std::string &filename);

public:
    explicit Frontend(Logger &logger) : logger{logger} { }
    ~Frontend() { this->logger.removeOutput(this->auxOutStream); }

    int casino(int argc, char **argv);
    int preview(int argc, char **argv);
    int trajectory(int argc, char **argv);
    int shapePreview(int argc, char **argv);

    int printGeneralHelp(const std::string &cmd);
};


#endif //RAMPACK_FRONTEND_H
