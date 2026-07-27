//
// Created by Codex on 18/07/2026.
//

#include "SelectionValidator.h"

#include <algorithm>
#include <sstream>

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

        SelectionValidationData(const ParticleSelection &selection, const std::size_t numParticles)
                : numParticles{numParticles}, numEligible{numParticles}, firstOutOfRange{numParticles}
        {
            const auto mode = selection.getMode();
            if (mode == ParticleSelection::Mode::ALL)
                return;

            const auto &indices = selection.getSpecifiedParticleIndices();
            const auto outOfRangeStart = std::lower_bound(indices.begin(), indices.end(), numParticles);
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
                    this->numEligible = numParticles - numInRange;
                    break;
                case ParticleSelection::Mode::ALL:
                    AssertThrow("unreachable");
                default:
                    AssertThrow("ParticleSelection::Mode");
            }
        }
    };

    void warnOutOfRangeIndices(const std::string &objectType, const std::string &objectName,
                               const SelectionValidationData &selectionData, Logger &logger)
    {
        if (selectionData.numOutOfRange == 0)
            return;

        const auto indexOrIndices = (selectionData.numOutOfRange == 1 ? "index" : "indices");
        logger.warn() << objectType << " '" << objectName << "' " << selectionData.maskArgumentName;
        logger << " contains " << selectionData.numOutOfRange << " out-of-range " << indexOrIndices;
        logger << " ignored for packing size " << selectionData.numParticles << ", starting at index ";
        logger << selectionData.firstOutOfRange << std::endl;
    }

    void validateEligibleParticles(const std::string &objectType, const std::string &objectName,
                                   const SelectionValidationData &selectionData)
    {
        if (selectionData.numEligible != 0)
            return;

        std::ostringstream message;
        message << objectType << " '" << objectName << "' has no eligible particles after applying ";
        message << selectionData.maskArgumentName << " for packing size " << selectionData.numParticles;
        throw ValidationException(message.str());
    }
}

void SelectionValidator::validate(const ParticleSelectable &selectable, const std::string &objectType,
                                  const std::string &objectName, const std::size_t numParticles, Logger &logger)
{
    Expects(numParticles != 0);

    const SelectionValidationData selectionData{selectable.getParticleSelection(), numParticles};
    warnOutOfRangeIndices(objectType, objectName, selectionData, logger);
    validateEligibleParticles(objectType, objectName, selectionData);
}
