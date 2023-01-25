//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_MODEBASE_H
#define RAMPACK_MODEBASE_H

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
class ModeBase {
protected:
    Logger &logger;
    std::ofstream auxOutStream;

    void setVerbosityLevel(std::optional<std::string> verbosity, std::optional<std::string> auxOutput,
                           std::optional<std::string> auxVerbosity);

    Logger::LogType parseVerbosityLevel(const std::string &verbosityLevelName) const;
    std::unique_ptr<RamtrjPlayer> loadRamtrjPlayer(std::string &trajectoryFilename, size_t numMolecules, bool autoFix_);
    static void createWalls(Packing &packing, const std::array<bool, 3> &walls);

    void storeSnapshots(const ObservablesCollector &observablesCollector, bool isContinuation,
                        const std::string &observableSnapshotFilename) const;

    void storeBulkObservables(const ObservablesCollector &observablesCollector,
                              std::string bulkObservableFilenamePattern) const;

    static void combineEnvironment(Simulation::Environment &env, const Run &run);
    RampackParameters dispatchParams(const std::string &filename);

public:
    explicit ModeBase(Logger &logger) : logger{logger} { }
    ~ModeBase() { this->logger.removeOutput(this->auxOutStream); }
};


#endif //RAMPACK_MODEBASE_H
