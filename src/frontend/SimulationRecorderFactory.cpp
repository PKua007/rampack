//
// Created by Piotr Kubala on 23/01/2023.
//

#include <fstream>

#include "SimulationRecorderFactory.h"
#include "utils/Exceptions.h"
#include "core/io/RamtrjRecorder.h"
#include "core/io/XYZRecorder.h"


std::unique_ptr<SimulationRecorder> RamtrjRecorderFactory::create(std::size_t numMolecules, std::size_t snapshotEvery,
                                                                  bool isContinuation, Logger &logger) const
{
    std::unique_ptr<std::fstream> inout;

    if (isContinuation) {
        inout = std::make_unique<std::fstream>(
            this->filename, std::ios_base::in | std::ios_base::out | std::ios_base::binary
        );
    } else {
        inout = std::make_unique<std::fstream>(
            this->filename, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc
        );
    }

    ValidateOpenedDesc(*inout, this->filename, "to store RAMTRJ trajectory");
    logger.info() << "RAMTRJ trajectory is stored on the fly to '" << this->filename << "'" << std::endl;
    return std::make_unique<RamtrjRecorder>(std::move(inout), numMolecules, snapshotEvery, isContinuation);
}

std::unique_ptr<SimulationRecorder> XYZRecorderFactory::create([[maybe_unused]] std::size_t numMolecules,
                                                               [[maybe_unused]] std::size_t snapshotEvery,
                                                               bool isContinuation, Logger &logger) const
{
    std::unique_ptr<std::fstream> inout;

    if (isContinuation) {
        inout = std::make_unique<std::fstream>(
            this->filename, std::ios_base::in | std::ios_base::out | std::ios_base::ate
        );
    } else {
        inout = std::make_unique<std::fstream>(
            this->filename,std::ios_base::in | std::ios_base::out | std::ios_base::trunc
        );
    }

    ValidateOpenedDesc(*inout, this->filename, "to store XYZ trajectory");
    logger.info() << "XYZ trajectory is stored on the fly to '" << this->filename << "'" << std::endl;
    return std::make_unique<XYZRecorder>(std::move(inout), isContinuation);
}
