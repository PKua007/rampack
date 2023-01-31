//
// Created by Piotr Kubala on 25/01/2023.
//

#include <fstream>
#include <regex>
#include <filesystem>

#include "IO.h"
#include "utils/Utils.h"
#include "matchers/RampackMatcher.h"
#include "legacy/Parameters.h"
#include "legacy/IniParametersFactory.h"


void IO::storeSnapshots(const ObservablesCollector &observablesCollector, bool isContinuation,
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

std::unique_ptr<RamtrjPlayer> IO::loadRamtrjPlayer(std::string &trajectoryFilename, std::size_t numMolecules,
                                                   bool autoFix_)
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
        ValidateMsg(player->getNumMolecules() == numMolecules,
                    "Number of molecules in input file and in the loaded trajectory are different");
        return player;
    }
}

void IO::storeBulkObservables(const ObservablesCollector &observablesCollector,
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

RampackParameters IO::dispatchParams(const std::string &filename) {
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

void IO::storeAverageValues(const std::string &filename, const ObservablesCollector &collector, double temperature,
                            double pressure) const
{
    auto flatValues = collector.getFlattenedAverageValues();

    std::ofstream out;
    if (!std::filesystem::exists(filename)) {
        out.open(filename);
        ValidateOpenedDesc(out, filename, "to store average values");
        out << "temperature pressure ";
        for (const auto &value : flatValues)
            out << value.name << " d" << value.name << " ";
        out << std::endl;
    } else {
        out.open(filename, std::ios_base::app);
        ValidateOpenedDesc(out, filename, "to store average values");
    }

    out.precision(std::numeric_limits<double>::max_digits10);
    out << temperature << " " << pressure << " ";
    for (auto &value : flatValues) {
        value.quantity.separator = Quantity::SPACE;
        out << value.quantity << " ";
    }
    out << std::endl;

    this->logger.info() << "Average values stored to " + filename << std::endl;
}
