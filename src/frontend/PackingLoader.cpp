//
// Created by pkua on 12.04.2022.
//

#include <fstream>
#include <iomanip>

#include "PackingLoader.h"
#include "core/RamsnapReader.h"


void PackingLoader::loadPacking(std::unique_ptr<BoundaryConditions> bc, const Interaction &interaction,
                                std::size_t moveThreads, std::size_t scalingThreads)
{
    this->reset();
    this->findStartRunIndex();

    if (this->isAllFinished_)
        return;

    if ((!this->startFrom.has_value() || this->startRunIndex == 0) && !this->continuationCycles.has_value())
        return;

    std::size_t startingPackingRunIndex{};
    if (this->continuationCycles.has_value())
        startingPackingRunIndex = this->startRunIndex;
    else
        startingPackingRunIndex = this->startRunIndex - 1;
    // A run, whose resulting packing will be the starting point
    auto &startingPackingRun = this->runsParameters[startingPackingRunIndex];

    auto packingFilenameGetter = [](auto &&run) { return run.packingFilename; };
    std::string previousPackingFilename = std::visit(packingFilenameGetter, startingPackingRun);
    std::ifstream packingFile(previousPackingFilename);
    ValidateOpenedDesc(packingFile, previousPackingFilename, "to load previous packing");
    // Same number of scaling and domain decemposition threads
    this->packing = std::make_unique<Packing>(std::move(bc), moveThreads, scalingThreads);
    this->auxInfo = this->packing->restore(packingFile, interaction);

    if (this->continuationCycles.has_value()) {
        this->cycleOffset = std::stoul(this->auxInfo.at("cycles"));
        this->isContinuation_ = true;

        // Value of continuation cycles is only used in integration mode. For overlaps rejection it is redundant
        if (std::holds_alternative<Parameters::IntegrationParameters>(startingPackingRun)) {
            // Because we continue this already finished run
            auto &startingRun = std::get<Parameters::IntegrationParameters>(startingPackingRun);

            if (this->continuationCycles == 0)
                this->continuationCycles = startingRun.thermalisationCycles;

            if (this->continuationCycles <= this->cycleOffset) {
                startingRun.thermalisationCycles = 0;
                this->logger.info() << "Thermalisation of the finished run '" << startingRun.runName;
                this->logger << "' will be skipped, since " << *this->continuationCycles << " or more cycles were ";
                this->logger << "already performed." << std::endl;
            } else {
                startingRun.thermalisationCycles = *this->continuationCycles - this->cycleOffset;
                this->logger.info() << "Thermalisation from the finished run '" << startingRun.runName;
                this->logger << "' will be continued up to " << *this->continuationCycles << " cycles (";
                this->logger << startingRun.thermalisationCycles << " to go)" << std::endl;
            }
        }
    }

    this->logger.info() << "Loaded packing from the run '" << previousPackingFilename << "' as a starting point.";
    this->logger << std::endl;

    this->isRestored_ = true;
}

void PackingLoader::findStartRunIndex() {
    if (!this->startFrom.has_value()) {
        this->startRunIndex = 0;
        return;
    }

    if (this->startFrom == ".auto") {
        this->autoFindStartRunIndex();
        return;
    }

    auto nameMatchesStartFrom = [this](const auto &params) {
        auto runNameGetter = [](auto &&run) { return run.runName; };
        return std::visit(runNameGetter, params) == this->startFrom;
    };
    auto it = std::find_if(this->runsParameters.begin(), this->runsParameters.end(), nameMatchesStartFrom);

    ValidateMsg(it != this->runsParameters.end(), "Invalid run name to start from");
    this->startRunIndex = it - this->runsParameters.begin();
}

void PackingLoader::reset() {
    this->cycleOffset = 0;  // Non-zero, if continuing previous run
    this->isContinuation_ = false;
    this->packing = nullptr;
    this->auxInfo.clear();
    this->isRestored_ = false;
    this->startRunIndex = 0;
    this->isAllFinished_ = false;
}

void PackingLoader::autoFindStartRunIndex() {
    ValidateMsg(this->allRunsHaveDatOutput(),
                "Starting run auto-detect: all runs must be set to output internal packing representation. Aborting.");

    std::vector<PerformedRunData> runDatas = this->gatherRunData();

    this->logRunsStatus(runDatas);
    this->warnIfOverlapRelaxation();

    auto isRunCorrupted = [](const auto &run) { return run.isCorrupted; };
    auto isRunFinished = [](const auto &run) { return run.isFinished(); };
    auto isRunPerformed = [](const auto &run) { return run.wasPerformed; };

    auto corruptedRun = std::find_if(runDatas.begin(), runDatas.end(), isRunCorrupted);
    if (corruptedRun != runDatas.end())
        throw ValidationException("Starting run auto-detect: one or more runs are corrupted. Aborting.");

    auto firstUnfinished = std::find_if_not(runDatas.begin(), runDatas.end(), isRunFinished);
    if (firstUnfinished == runDatas.end()) {
        this->logger.warn() << "Starting run auto-detect: all runs were finished.";
        this->logger << std::endl;

        this->continuationCycles = std::nullopt;
        this->startRunIndex = runDatas.size();
        this->isAllFinished_ = true;
        return;
    }

    auto unexpectedPerformed = std::find_if(std::next(firstUnfinished), runDatas.end(), isRunPerformed);
    if (unexpectedPerformed != runDatas.end()) {
        throw ValidationException("Starting run auto-detect: detected already performed run: '"
                                  + unexpectedPerformed->runName + "' after unfinished run: '"
                                  + firstUnfinished->runName + "'. Aborting.");
    }

    this->logger.info() << "Starting run auto-detect: detected '" <<  firstUnfinished->runName;
    this->logger << "' as a first unfinished run." << std::endl;

    if (firstUnfinished->doneCycles == 0)
        this->continuationCycles = std::nullopt;
    else
        this->continuationCycles = 0;

    this->startRunIndex = firstUnfinished - runDatas.begin();
}

void PackingLoader::warnIfOverlapRelaxation() const {
    auto isOverlapReduction = [](const auto &params) {
        return std::holds_alternative<Parameters::OverlapRelaxationParameters>(params);
    };
    if (std::find_if(runsParameters.begin(), runsParameters.end(), isOverlapReduction) != runsParameters.end()) {
        this->logger.warn() << "Starting run auto-detect: some runs are overlap relaxation; auto-detection may fail";
        this->logger << std::endl;
    }
}

bool PackingLoader::allRunsHaveDatOutput() const {
    auto getDatOutput = [](const auto &params) {
        auto packingFilenameGetter = [](auto &&run) { return run.packingFilename; };
        return std::visit(packingFilenameGetter, params);
    };
    auto hasDatOutput = [getDatOutput](const auto &params) { return !getDatOutput(params).empty(); };
    return std::all_of(runsParameters.begin(), runsParameters.end(), hasDatOutput);
}

void PackingLoader::logRunsStatus(const std::vector<PerformedRunData> &runDatas) const {
    auto runNameLengthComp = [](const auto &run1, const auto &run2) {
        return run1.runName.length() < run2.runName.length();
    };
    std::size_t longestName = std::max_element(runDatas.begin(), runDatas.end(), runNameLengthComp)->runName.length();

    this->logger.info() << "Starting run auto-detect: status of runs:" << std::endl;
    for (const auto &runData : runDatas) {
        this->logger << std::left << std::setw(static_cast<int>(longestName)) << runData.runName << " : ";
        if (runData.isCorrupted) {
            this->logger << "corrupted" << std::endl;
        } else if (runData.wasPerformed) {
            this->logger << (runData.isFinished() ? "finished     " : "unfinished   ");
            this->logger << " (" << runData.doneCycles << "/" << runData.expectedCycles << " cycles)" << std::endl;
        } else {
            this->logger << "not performed" << std::endl;
        }
    }
}

std::vector<PackingLoader::PerformedRunData> PackingLoader::gatherRunData() const {
    std::vector<PerformedRunData> runDatas;
    runDatas.reserve(runsParameters.size());
    for (const auto &runParams : runsParameters) {
        PerformedRunData runData;
        auto runNameGetter = [](auto &&run) { return run.runName; };
        runData.runName = std::visit(runNameGetter, runParams);

        if (std::holds_alternative<Parameters::IntegrationParameters>(runParams)) {
            const auto &integrationParams = std::get<Parameters::IntegrationParameters>(runParams);
            runData.expectedCycles = integrationParams.thermalisationCycles + integrationParams.averagingCycles;
        } else {
            runData.expectedCycles = 0;
        }

        auto packingFilenameGetter = [](auto &&run) { return run.packingFilename; };
        auto packingFilename = std::visit(packingFilenameGetter, runParams);
        std::ifstream datInput(packingFilename);
        if (datInput) {
            runData.wasPerformed = true;
            try {
                auto auxInfo_ = RamsnapReader::restoreAuxInfo(datInput);
                Validate(auxInfo_.find("cycles") != auxInfo_.end());
                runData.doneCycles = std::stoul(auxInfo_.at("cycles"));
            } catch (std::logic_error &) {
                runData.isCorrupted = true;
            }
        } else {
            runData.wasPerformed = false;
        }

        runDatas.push_back(runData);
    }

    return runDatas;
}
