//
// Created by Codex on 04/07/2026.
//

#include "MoveSelectionValidator.h"

#include <algorithm>
#include <sstream>
#include <string>

#include "core/MoveSampler.h"
#include "core/ParticleSelection.h"
#include "utils/Exceptions.h"
#include "utils/Logger.h"


namespace {
    struct SelectionValidationData {
        std::size_t numParticles{};
        std::size_t numOutOfRange{};
        std::size_t numEligible{};
        std::size_t firstOutOfRange{};
        std::string maskArgumentName{};

        SelectionValidationData(const ParticleSelection &selection_, const std::size_t numParticles_)
                : numParticles{numParticles_}, numEligible{numParticles_}, firstOutOfRange{numParticles_}
        {
            const auto mode = selection_.getMode();
            if (mode == ParticleSelection::Mode::ALL)
                return;

            const auto &indices = selection_.getSpecifiedParticleIndices();
            const auto outOfRangeStart = std::lower_bound(indices.begin(), indices.end(), this->numParticles);

            const auto numInRange = static_cast<std::size_t>(std::distance(indices.begin(), outOfRangeStart));
            this->numOutOfRange = static_cast<std::size_t>(std::distance(outOfRangeStart, indices.end()));

            if (this->numOutOfRange > 0)
                this->firstOutOfRange = *outOfRangeStart;

            switch (mode) {
                case ParticleSelection::Mode::WHITELIST:
                    this->maskArgumentName = "whitelist";
                    this->numEligible = numInRange;
                    break;
                case ParticleSelection::Mode::BLACKLIST:
                    this->maskArgumentName = "blacklist";
                    this->numEligible = this->numParticles - numInRange;
                    break;
                case ParticleSelection::Mode::ALL:
                    AssertThrow("unreachable");
                default:
                    AssertThrow("ParticleSelection::Mode");
            }
        }
    };

    void warnOutOfRangeIndices(const std::string &moveSamplerName, const SelectionValidationData &selectionData,
                               Logger &logger)
    {
        if (selectionData.numOutOfRange == 0)
            return;

        const auto indexOrIndices = (selectionData.numOutOfRange == 1 ? "index" : "indices");
        logger.warn() << "Move sampler '" << moveSamplerName << "' " << selectionData.maskArgumentName;
        logger << " contains " << selectionData.numOutOfRange << " out-of-range " << indexOrIndices;
        logger << " ignored for packing size " << selectionData.numParticles << ", starting at index ";
        logger << selectionData.firstOutOfRange;
        logger << std::endl;
    }

    void validateEligibleParticles(const std::string &moveSamplerName, const SelectionValidationData &selectionData) {
        if (selectionData.numEligible != 0)
            return;

        std::ostringstream message;
        message << "Move sampler '" << moveSamplerName << "' has no eligible particles after applying ";
        message << selectionData.maskArgumentName << " for packing size ";
        message << selectionData.numParticles;
        throw ValidationException(message.str());
    }

    void validateSamplerSelection(const MoveSampler &moveSampler, const std::size_t numParticles, Logger &logger) {
        SelectionValidationData selectionData{moveSampler.getParticleSelection(), numParticles};
        const auto moveSamplerName = moveSampler.getName();

        warnOutOfRangeIndices(moveSamplerName, selectionData, logger);
        validateEligibleParticles(moveSamplerName, selectionData);
    }
}

void MoveSelectionValidator::validate(const Simulation::Environment &environment, const std::size_t numParticles,
                                      Logger &logger)
{
    Expects(numParticles != 0);

    for (const auto &moveSampler : environment.getMoveSamplers())
        validateSamplerSelection(*moveSampler, numParticles, logger);
}
