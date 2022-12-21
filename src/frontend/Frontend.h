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
#include "core/io/RamtrjRecorder.h"
#include "core/io/RamtrjPlayer.h"
#include "PackingLoader.h"
#include "core/io/XYZRecorder.h"
#include "utils/Version.h"


/**
 * @brief Class responsible for the communication between the user and the simulation backend.
 */
class Frontend {
private:
    Logger &logger;
    std::ofstream auxOutStream;

    Parameters loadParameters(const std::string &inputFilename);
    std::unique_ptr<Packing> recreatePacking(PackingLoader &loader, const Parameters &params,
                                             const ShapeTraits &traits, std::size_t maxThreads);
    void overwriteMoveStepSizes(Simulation::Environment &env,
                                const std::map<std::string, std::string> &packingAuxInfo) const;
    Simulation::Environment recreateEnvironment(const Parameters &params, const PackingLoader &loader,
                                                const ShapeTraits &traits) const;
    Simulation::Environment recreateRawEnvironment(const Parameters &params, std::size_t startRunIndex,
                                                   const ShapeTraits &traits) const;

    void setVerbosityLevel(std::optional<std::string> verbosity, std::optional<std::string> auxOutput,
                           std::optional<std::string> auxVerbosity);
    void generateRamsnapFile(const Packing &packing, const std::string &ramsnapFilename, std::size_t cycles = 0);
    void generateXYZFile(const Packing &packing, const ShapeTraits &traits, const std::string &ramsnapFilename,
                         std::size_t cycles = 0);
    Logger::LogType parseVerbosityLevel(const std::string &verbosityLevelName) const;
    std::unique_ptr<RamtrjRecorder> loadRamtrjRecorder(const std::string &filename, std::size_t numMolecules,
                                                       std::size_t cycleStep, bool isContinuation) const;
    std::unique_ptr<XYZRecorder> loadXYZRecorder(const std::string &filename, bool isContinuation) const;
    std::unique_ptr<RamtrjPlayer> loadRamtrjPlayer(std::string &trajectoryFilename, size_t numMolecules, bool autoFix_);
    void createWalls(Packing &packing, const std::string &walls);
    void attachSnapshotOut(ObservablesCollector &collector, const std::string& filename, bool isContinuation) const;

    void verifyDynamicParameter(const DynamicParameter &dynamicParameter, const std::string &parameterName,
                                const Parameters::IntegrationParameters &params, std::size_t cycleOffset) const;

    void performIntegration(Simulation &simulation, Simulation::Environment &env,
                            const Parameters::IntegrationParameters &runParams, const ShapeTraits &shapeTraits,
                            size_t cycleOffset, bool isContinuation);

    void performOverlapRelaxation(Simulation &simulation, Simulation::Environment &envfco,
                                  const std::string &shapeName, const std::string &shapeAttr,
                                  const Parameters::OverlapRelaxationParameters &runParams,
                                  std::shared_ptr<ShapeTraits> shapeTraits, size_t cycleOffset, bool isContinuation,
                                  const Version &paramsVersion);

    void printPerformanceInfo(const Simulation &simulation);
    void printAverageValues(const ObservablesCollector &collector);

    [[nodiscard]] std::pair<std::string, std::map<std::string, std::string>>
    parseFilenameAndParams(const std::string &str, const std::vector<std::string>& fields) const;

    void storeAverageValues(const std::string &filename, const ObservablesCollector &collector, double temperature,
                            double pressure) const;
    [[nodiscard]] std::map<std::string, std::string> prepareAuxInfo(const Simulation &simulation) const;
    void storeRamsnap(const Simulation &simulation, const std::string &packingFilename);
    void storeXYZ(const Simulation &simulation, const ShapeTraits &traits, const std::string &packingFilename);
    void storeWolframVisualization(const Packing &packing, const ShapeTraits &traits,
                                   const std::string &wolframAttr) const;
    void storeSnapshots(const ObservablesCollector &observablesCollector, bool isContinuation,
                        const std::string &observableSnapshotFilename) const;
    void storeBulkObservables(const ObservablesCollector &observablesCollector,
                              std::string bulkObservableFilenamePattern) const;

    void printMoveStatistics(const Simulation &simulation) const;

    void printInteractionInfo(const Interaction &interaction);
    void printGeometryInfo(const ShapeGeometry &geometry);

    static std::string doubleToString(double d);
    static std::string formatMoveKey(const std::string &groupName, const std::string &moveName);
    static bool isStepSizeKey(const std::string &key);
    static Simulation::Environment parseSimulationEnvironment(const InheritableParameters &params,
                                                              const ShapeTraits &traits);
    static void combineEnvironment(Simulation::Environment &env, const Parameters::RunParameters &runParams,
                                   const ShapeTraits &traits);

public:
    explicit Frontend(Logger &logger) : logger{logger} { }
    ~Frontend() { this->logger.removeOutput(this->auxOutStream); }

    int casino(int argc, char **argv);
    int optimize_distance(int argc, char **argv);
    int preview(int argc, char **argv);
    int trajectory(int argc, char **argv);
    int shapePreview(int argc, char **argv);

    int printGeneralHelp(const std::string &cmd);

    std::shared_ptr<ShapeTraits>
    createShapeTraits(const std::string &shapeName, const std::string &shapeAttributes, const std::string &interaction,
                      Version version);
};


#endif //RAMPACK_FRONTEND_H
