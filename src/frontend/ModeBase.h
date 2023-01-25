//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_MODEBASE_H
#define RAMPACK_MODEBASE_H

#include <vector>
#include <memory>
#include <fstream>

#include "IO.h"
#include "RampackParameters.h"
#include "core/BoundaryConditions.h"
#include "core/Simulation.h"
#include "utils/Logger.h"


/**
 * @brief Class responsible for the communication between the user and the simulation backend.
 */
class ModeBase {
protected:
    Logger &logger;
    IO io;
    std::ofstream auxOutStream;

    void setVerbosityLevel(std::optional<std::string> verbosity, std::optional<std::string> auxOutput,
                           std::optional<std::string> auxVerbosity);
    Logger::LogType parseVerbosityLevel(const std::string &verbosityLevelName) const;
    static void createWalls(Packing &packing, const std::array<bool, 3> &walls);
    static void combineEnvironment(Simulation::Environment &env, const Run &run);

public:
    explicit ModeBase(Logger &logger) : logger{logger}, io(logger) { }
    ~ModeBase() { this->logger.removeOutput(this->auxOutStream); }
};


#endif //RAMPACK_MODEBASE_H
