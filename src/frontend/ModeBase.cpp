//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cstdlib>
#include <limits>
#include <iterator>
#include <filesystem>
#include <set>

#include <cxxopts.hpp>

#include "ModeBase.h"
#include "RampackParameters.h"
#include "core/Simulation.h"
#include "core/Packing.h"
#include "matchers/RampackMatcher.h"
#include "legacy/IniParametersFactory.h"
#include "utils/Fold.h"
#include "utils/Utils.h"
#include "utils/Assertions.h"


void ModeBase::setVerbosityLevel(std::optional<std::string> verbosity, std::optional<std::string> auxOutput,
                                 std::optional<std::string> auxVerbosity)
{
    if (verbosity.has_value())
        this->logger.setVerbosityLevel(this->parseVerbosityLevel(*verbosity));
    else if (auxOutput.has_value())
        this->logger.setVerbosityLevel(Logger::WARN);
    else
        this->logger.setVerbosityLevel(Logger::INFO);

    if (!auxOutput.has_value())
        return;

    this->auxOutStream.open(*auxOutput);
    ValidateOpenedDesc(this->auxOutStream, *auxOutput, "to log messages");

    this->logger.warn() << "Logging output to '" << *auxOutput << "'" << std::endl;

    this->logger.addOutput(this->auxOutStream);

    if (auxVerbosity.has_value())
        this->logger.setVerbosityLevel(this->parseVerbosityLevel(*auxVerbosity), this->auxOutStream);
    else
        this->logger.setVerbosityLevel(Logger::INFO, this->auxOutStream);
}

Logger::LogType ModeBase::parseVerbosityLevel(const std::string &verbosityLevelName) const {
    if (verbosityLevelName == "error")
        return Logger::ERROR;
    else if (verbosityLevelName == "warn")
        return Logger::WARN;
    else if (verbosityLevelName == "info")
        return Logger::INFO;
    else if (verbosityLevelName == "verbose")
        return Logger::VERBOSE;
    else if (verbosityLevelName == "debug")
        return Logger::DEBUG;
    else
        die("Unknown verbosity level: " + verbosityLevelName, this->logger);

    return Logger::ERROR;
}


void ModeBase::combineEnvironment(Simulation::Environment &env, const Run &run)
{
    auto environmentGetter = [](const auto &run) {
        return run.environment;
    };
    auto runEnv = std::visit(environmentGetter, run);
    env.combine(runEnv);
}

void ModeBase::storeSnapshots(const ObservablesCollector &observablesCollector, bool isContinuation,
                              const std::string &observableSnapshotFilename) const
{
    if (isContinuation) {
        std::ofstream out(observableSnapshotFilename, std::ios_base::app);
        ValidateOpenedDesc(out, observableSnapshotFilename, "to store observables");
        observablesCollector.printSnapshots(out, false);
    } else {
        std::ofstream out(observableSnapshotFilename);
        ValidateOpenedDesc(out, observableSnapshotFilename, "to store observables");
        observablesCollector.printSnapshots(out, true);
    }

    this->logger.info() << "Observable snapshots stored to " + observableSnapshotFilename << std::endl;
}

std::unique_ptr<RamtrjPlayer> ModeBase::loadRamtrjPlayer(std::string &trajectoryFilename,
                                                         std::size_t numMolecules, bool autoFix_)
{
    auto trajectoryStream = std::make_unique<std::ifstream>(trajectoryFilename,
                                                            std::ios_base::in | std::ios_base::binary);
    ValidateOpenedDesc(*trajectoryStream, trajectoryFilename, "to read trajectory");
    if (autoFix_) {
        RamtrjPlayer::AutoFix autoFix(numMolecules);
        try {
            auto simulationPlayer = std::make_unique<RamtrjPlayer>(std::move(trajectoryStream), autoFix);
            autoFix.dumpInfo(this->logger);
            this->logger.info() << std::endl;
            return simulationPlayer;
        } catch (const ValidationException &exception) {
            autoFix.dumpInfo(this->logger);
            return nullptr;
        }
    } else {
        std::unique_ptr<RamtrjPlayer> player;
        try {
            player = std::make_unique<RamtrjPlayer>(std::move(trajectoryStream));
        } catch (const ValidationException &exception) {
            this->logger.error() << "Reading the trajectory failed: " << exception.what() << std::endl;
            this->logger << "You may try to fix it by adding the --auto-fix option." << std::endl;
            return nullptr;
        }
        Validate(player->getNumMolecules() == numMolecules);
        return player;
    }
}

void ModeBase::createWalls(Packing &packing, const std::array<bool, 3> &walls) {
    for (std::size_t i{}; i < 3; i++)
        packing.toggleWall(i, walls[i]);
}

void ModeBase::storeBulkObservables(const ObservablesCollector &observablesCollector,
                                    std::string bulkObservableFilenamePattern) const
{
    std::regex pattern(R"(\{\})");
    if (!std::regex_search(bulkObservableFilenamePattern, pattern))
        bulkObservableFilenamePattern = bulkObservableFilenamePattern + "_{}.txt";

    observablesCollector.visitBulkObservables([&](const BulkObservable &bulkObservable) {
        std::string observableName = bulkObservable.getSignatureName();
        std::string filename = std::regex_replace(bulkObservableFilenamePattern, pattern, observableName);
        std::ofstream out(filename);
        ValidateOpenedDesc(out, filename, "to store bulk observable");
        bulkObservable.print(out);
        this->logger.info() << "Bulk observable " << observableName << " stored to " << filename << std::endl;
    });
}

RampackParameters ModeBase::dispatchParams(const std::string &filename) {
    std::ifstream file(filename);
    ValidateOpenedDesc(file, filename, "to load the simulation script");

    std::string firstLine;
    std::getline(file, firstLine);
    trim(firstLine);
    file.seekg(0, std::ios::beg);

    if (startsWith(firstLine, "rampack")) {
        this->logger.info() << "Parameters format in '" << filename << "' recognized as: PYON" << std::endl;
        std::ostringstream fileContent;
        fileContent << file.rdbuf();
        return RampackMatcher::match(fileContent.str());
    } else {
        this->logger.info() << "Parameters format in '" << filename << "' recognized as: INI" << std::endl;
        Parameters params(file);
        return IniParametersFactory::create(params);
    }
}