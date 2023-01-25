//
// Created by Piotr Kubala on 25/01/2023.
//

#ifndef RAMPACK_IO_H
#define RAMPACK_IO_H

#include "utils/Logger.h"
#include "RampackParameters.h"
#include "core/io/RamtrjPlayer.h"


class IO {
private:
    Logger &logger;

public:
    explicit IO(Logger &logger) : logger{logger} { }

    RampackParameters dispatchParams(const std::string &filename);
    std::unique_ptr<RamtrjPlayer> loadRamtrjPlayer(std::string &trajectoryFilename, size_t numMolecules, bool autoFix_);

    void storeSnapshots(const ObservablesCollector &observablesCollector, bool isContinuation,
                        const std::string &observableSnapshotFilename) const;

    void storeBulkObservables(const ObservablesCollector &observablesCollector,
                              std::string bulkObservableFilenamePattern) const;
};


#endif //RAMPACK_IO_H
