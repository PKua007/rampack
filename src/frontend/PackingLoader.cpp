//
// Created by pkua on 12.04.2022.
//

#include <fstream>
#include <iomanip>

#include "PackingLoader.h"
#include "core/io/RamsnapReader.h"


void PackingLoader::loadPacking(std::unique_ptr<BoundaryConditions> bc, const Interaction &interaction,
                                const ShapeDataManager &dataManager, std::size_t moveThreads,
                                std::size_t scalingThreads)
{
    this->reset();

    this->findStartRunIndex();
    if (this->isAllFinished_)
        return;

    if (this->isStartingFromScratch())
        return;

    if (this->continuationCycles.has_value())
        this->loadPackingContinuation(std::move(bc), interaction, dataManager, moveThreads, scalingThreads);
    else
        this->loadPackingNoContinuation(std::move(bc), interaction, dataManager, moveThreads, scalingThreads);
}

bool PackingLoader::isStartingFromScratch() const {
    return (!startFrom.has_value() || startRunIndex == 0) && !continuationCycles.has_value();
}

void PackingLoader::restorePacking(const Run &startingPackingRun, std::unique_ptr<BoundaryConditions> bc,
                                   const Interaction &interaction, const ShapeDataManager &dataManager,
                                   std::size_t moveThreads, std::size_t scalingThreads)
{
    auto packingFilenameGetter = [](auto &&run) { return *run.ramsnapOut; };
    std::string startingPackingFilename = std::visit(packingFilenameGetter, startingPackingRun);
    std::ifstream packingFile(startingPackingFilename);
    ValidateOpenedDesc(packingFile, startingPackingFilename, "to load initial packing");

    this->packing = std::make_unique<Packing>(std::move(bc), moveThreads, scalingThreads);
    this->auxInfo = this->packing->restore(packingFile, interaction, dataManager);
    this->isRestored_ = true;

    auto runNameGetter = [](auto &&run) { return run.runName; };
    std::string startingRunName = std::visit(runNameGetter, startingPackingRun);
    this->logger.info() << "Loaded packing '" << startingPackingFilename << "' from the run '" << startingRunName;
    this->logger << "' as a starting point." << std::endl;
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

    this->startRunIndex = PackingLoader::findStartRunIndex(*this->startFrom, this->runsParameters);
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
        this->logger.info() << "Starting run auto-detect: all runs were finished.";
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
        return std::holds_alternative<OverlapRelaxationRun>(params);
    };
    if (std::find_if(runsParameters.begin(), runsParameters.end(), isOverlapReduction) != runsParameters.end()) {
        this->logger.warn() << "Starting run auto-detect: some runs are overlap relaxation; auto-detection may fail";
        this->logger << std::endl;
    }
}

bool PackingLoader::allRunsHaveDatOutput() const {
    auto hasDatOutput = [](const auto &params) {
        auto ramsnapOutGetter = [](auto &&run) { return run.ramsnapOut; };
        return std::visit(ramsnapOutGetter, params).has_value();
    };
    return std::all_of(this->runsParameters.begin(), this->runsParameters.end(), hasDatOutput);
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

        if (std::holds_alternative<IntegrationRun>(runParams)) {
            const auto &integrationRun = std::get<IntegrationRun>(runParams);
            runData.expectedCycles = integrationRun.thermalizationCycles.value_or(0)
                                     + integrationRun.averagingCycles.value_or(0);
        } else {
            runData.expectedCycles = 0;
        }

        auto packingFilenameGetter = [](auto &&run) { return *run.ramsnapOut; };
        auto packingFilename = std::visit(packingFilenameGetter, runParams);
        std::ifstream ramsnapInput(packingFilename);
        if (ramsnapInput) {
            runData.wasPerformed = true;
            try {
                auto auxInfo_ = RamsnapReader::restoreAuxInfo(ramsnapInput);
                ValidateMsg(auxInfo_.find("cycles") != auxInfo_.end(), "No 'cycles' key in RAMSNAP metadata");
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

std::size_t PackingLoader::findStartRunIndex(const std::string &runName, const std::vector<Run> &runsParameters)
{
    if (runName == ".first")
        return 0;

    if (runName == ".last")
        return runsParameters.size() - 1;

    auto nameMatchesStartFrom = [&runName](const auto &params) {
        auto runNameGetter = [](auto &&run) { return run.runName; };
        return std::visit(runNameGetter, params) == runName;
    };
    auto it = std::find_if(runsParameters.begin(), runsParameters.end(), nameMatchesStartFrom);

    ValidateMsg(it != runsParameters.end(), "Invalid run name to start from");
    return it - runsParameters.begin();
}

void PackingLoader::loadPackingContinuation(std::unique_ptr<BoundaryConditions> bc, const Interaction &interaction,
                                            const ShapeDataManager &dataManager, std::size_t moveThreads,
                                            std::size_t scalingThreads)
{
    auto &startRun = this->runsParameters[this->startRunIndex];
    this->restorePacking(startRun, std::move(bc), interaction, dataManager, moveThreads, scalingThreads);

    this->cycleOffset = std::stoul(this->auxInfo.at("cycles"));
    this->isContinuation_ = true;

    // Value of continuation cycles is only used in integration mode. For overlaps rejection it is redundant
    if (!std::holds_alternative<IntegrationRun>(startRun))
        return;

    auto &integrationStartRun = std::get<IntegrationRun>(startRun);

    if (this->continuationCycles == 0)
        this->continuationCycles = integrationStartRun.thermalizationCycles.value_or(0);

    if (this->continuationCycles <= this->cycleOffset) {
        integrationStartRun.thermalizationCycles = std::nullopt;
        this->logger.info() << "Thermalization of the finished run '" << integrationStartRun.runName;
        this->logger << "' will be skipped, since " << *this->continuationCycles << " or more cycles were ";
        this->logger << "already performed." << std::endl;

        if (integrationStartRun.averagingCycles > 0)
            return;

        this->logger << "Averaging phase is turned off, moving to the next run." << std::endl;

        this->isContinuation_ = false;
        this->cycleOffset = 0;
        this->startRunIndex++;

        if (this->startRunIndex == this->runsParameters.size())
            this->isAllFinished_ = true;
    } else {
        integrationStartRun.thermalizationCycles = *this->continuationCycles - this->cycleOffset;
        this->logger.info() << "Thermalization from the finished run '" << integrationStartRun.runName;
        this->logger << "' will be continued up to " << *this->continuationCycles << " cycles (";
        this->logger << *integrationStartRun.thermalizationCycles << " to go)" << std::endl;
    }

}

void PackingLoader::loadPackingNoContinuation(std::unique_ptr<BoundaryConditions> bc, const Interaction &interaction,
                                              const ShapeDataManager &dataManager, std::size_t moveThreads,
                                              std::size_t scalingThreads)
{
    auto &startingPackingRun = this->runsParameters[this->startRunIndex - 1];
    this->restorePacking(startingPackingRun, std::move(bc), interaction, dataManager, moveThreads, scalingThreads);
}