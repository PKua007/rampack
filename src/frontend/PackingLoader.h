//
// Created by pkua on 12.04.2022.
//

#ifndef RAMPACK_PACKINGLOADER_H
#define RAMPACK_PACKINGLOADER_H

#include <memory>
#include <optional>
#include <utility>

#include "core/Packing.h"
#include "Parameters.h"


class PackingLoader {
private:
    struct PerformedRunData {
        std::string runName;
        std::size_t expectedCycles{};
        bool wasPerformed{};
        bool isCorrupted{};
        std::size_t doneCycles{};

        [[nodiscard]] bool isFinished() const {
            return this->wasPerformed && !this->isCorrupted && this->doneCycles >= this->expectedCycles;
        }
    };

    Logger &logger;
    std::optional<std::string> startFrom;
    std::optional<std::size_t> continuationCycles;
    std::vector<Parameters::RunParameters> &runsParameters;

    std::size_t startRunIndex{};
    std::size_t cycleOffset{};
    bool isContinuation_{};
    std::map<std::string, std::string> auxInfo{};
    bool isRestored_{};
    std::unique_ptr<Packing> packing{};
    bool isAllFinished_{};

    void findStartRunIndex();
    void autoFindStartRunIndex();
    [[nodiscard]] std::vector<PerformedRunData> gatherRunData() const;
    void logRunsStatus(const std::vector<PerformedRunData> &runDatas) const;
    [[nodiscard]] bool allRunsHaveDatOutput() const;
    void warnIfOverlapRelaxation() const;

public:
    PackingLoader(Logger &logger, std::optional<std::string> startFrom, std::optional<std::size_t> continuationCycles,
                  std::vector<Parameters::RunParameters> &runsParameters)
            : logger{logger}, startFrom{std::move(startFrom)}, continuationCycles{continuationCycles},
              runsParameters{runsParameters}
    { }

    void loadPacking(std::unique_ptr<BoundaryConditions> bc, const Interaction &interaction, std::size_t moveThreads,
                     std::size_t scalingThreads);

    [[nodiscard]] std::size_t getStartRunIndex() const { return this->startRunIndex; }
    [[nodiscard]] std::size_t getCycleOffset() const { return this->cycleOffset; }
    [[nodiscard]] bool isContinuation() const { return this->isContinuation_; }
    [[nodiscard]] const std::map<std::string, std::string> &getAuxInfo() const { return this->auxInfo; }
    [[nodiscard]] bool isRestored() const { return this->isRestored_; }
    [[nodiscard]] std::unique_ptr<Packing> releasePacking() { return std::move(this->packing); }
    [[nodiscard]] bool isAllFinished() const { return this->isAllFinished_; }

    void reset();
};


#endif //RAMPACK_PACKINGLOADER_H
