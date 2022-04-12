//
// Created by pkua on 12.04.2022.
//

#include <fstream>

#include "PackingLoader.h"


std::unique_ptr<Packing> PackingLoader::loadPacking(std::unique_ptr<BoundaryConditions> bc,
                                                    const Interaction &interaction, std::size_t moveThreads,
                                                    std::size_t scalingThreads)
{
    std::unique_ptr<Packing> packing;

    // Find starting run index if specified
    if (this->startFrom.has_value()) {
        auto nameMatchesStartFrom = [this](const auto &params) {
            auto runNameGetter = [](auto &&run) { return run.runName; };
            return std::visit(runNameGetter, params) == this->startFrom;
        };
        auto it = std::find_if(this->runsParameters.begin(), this->runsParameters.end(), nameMatchesStartFrom);

        ValidateMsg(it != this->runsParameters.end(), "Invalid run name to start from");
        this->startRunIndex = it - this->runsParameters.begin();
    }

    this->cycleOffset = 0;  // Non-zero, if continuing previous run
    this->isContinuation_ = false;
    if ((this->startFrom.has_value() && this->startRunIndex != 0) || this->continuationCycles.has_value()) {
        std::size_t startingPackingRunIndex{};
        if (this->continuationCycles.has_value())
            startingPackingRunIndex = startRunIndex;
        else
            startingPackingRunIndex = startRunIndex - 1;
        // A run, whose resulting packing will be the starting point
        auto &startingPackingRun = runsParameters[startingPackingRunIndex];

        auto packingFilenameGetter = [](auto &&run) { return run.packingFilename; };
        std::string previousPackingFilename = std::visit(packingFilenameGetter, startingPackingRun);
        std::ifstream packingFile(previousPackingFilename);
        ValidateOpenedDesc(packingFile, previousPackingFilename, "to load previous packing");
        // Same number of scaling and domain decemposition threads
        packing = std::make_unique<Packing>(std::move(bc), moveThreads, scalingThreads);
        this->auxInfo = packing->restore(packingFile, interaction);

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
    }

    return packing;
}
