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
#include "RampackParameters.h"


/**
 * @brief Class responsible for the communication between the user and the simulation backend.
 */
class Frontend {
private:
    Logger &logger;
    std::ofstream auxOutStream;

    Parameters loadParameters(const std::string &inputFilename);
    std::unique_ptr<Packing> recreatePacking(PackingLoader &loader, const BaseParameters &params,
                                             const ShapeTraits &traits, std::size_t maxThreads);
    std::unique_ptr<Packing> arrangePacking(std::size_t numOfParticles, const std::string &initialDimensions,
                                            const std::string &initialArrangement, const ShapeTraits &shapeTraits,
                                            std::size_t moveThreads, std::size_t scalingThreads,
                                            const Version &paramsVersion) const;
    void overwriteMoveStepSizes(Simulation::Environment &env,
                                const std::map<std::string, std::string> &packingAuxInfo) const;
    Simulation::Environment recreateEnvironment(const Parameters &params, const PackingLoader &loader) const;
    Simulation::Environment recreateEnvironment(const RampackParameters &params, const PackingLoader &loader) const;
    Simulation::Environment recreateRawEnvironment(const Parameters &params, std::size_t startRunIndex) const;

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
    void createWalls(Packing &packing, const std::array<bool, 3> &walls);
    void createWalls(Packing &packing, const std::string &walls);
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
                                                               const Version &paramsVersion);
    static void combineEnvironment(Simulation::Environment &env, const Parameters::RunParameters &runParams,
                                   const Version &paramsVersion);
    static void combineEnvironment(Simulation::Environment &env, const Run &run);

    [[nodiscard]] std::shared_ptr<ShapeTraits> createShapeTraits(const std::string &shapeName,
                                                                 const std::string &shapeAttributes,
                                                                 const std::string &interaction,
                                                                 const Version &paramsVersion) const;

    [[nodiscard]] std::shared_ptr<ObservablesCollector>
    createObservablesCollector(std::optional<std::string> observablesStr, std::optional<std::string> bulkObservablesStr,
                               std::size_t maxThreads, const Version &paramsVersion) const;

    static std::vector<std::shared_ptr<MoveSampler>> createMoveSamplers(const std::string &moveTypes,
                                                                        const Version &paramsVersion);

    static std::shared_ptr<TriclinicBoxScaler> createBoxScaler(const std::string &scalerStr, double volumeStepSize,
                                                               const Version &paramsVersion);


    static std::shared_ptr<DynamicParameter> createDynamicParameter(const std::string &parameterStr,
                                                                    const Version &paramsVersion);

    RampackParameters dispatchParams(const std::string &filename);

public:
    explicit Frontend(Logger &logger) : logger{logger} { }
    ~Frontend() { this->logger.removeOutput(this->auxOutStream); }

    int casino(int argc, char **argv);
    int optimize_distance(int argc, char **argv);
    int preview(int argc, char **argv);
    int trajectory(int argc, char **argv);
    int shapePreview(int argc, char **argv);

    int printGeneralHelp(const std::string &cmd);
};


#endif //RAMPACK_FRONTEND_H
