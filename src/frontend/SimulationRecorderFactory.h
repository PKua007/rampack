//
// Created by Piotr Kubala on 23/01/2023.
//

#ifndef RAMPACK_SIMULATIONRECORDERFACTORY_H
#define RAMPACK_SIMULATIONRECORDERFACTORY_H

#include <string>
#include <memory>

#include "core/SimulationRecorder.h"
#include "utils/Logger.h"


class SimulationRecorderFactory {
public:
    virtual ~SimulationRecorderFactory() = default;

    [[nodiscard]] virtual std::unique_ptr<SimulationRecorder> create(std::size_t numMolecules,
                                                                     std::size_t snapshotEvery,
                                                                     bool isContinuation, Logger &logger) const = 0;
};


class RamtrjRecorderFactory : public SimulationRecorderFactory {
private:
    std::string filename;

public:
    explicit RamtrjRecorderFactory(std::string filename) : filename{std::move(filename)}
    { }

    [[nodiscard]] std::unique_ptr<SimulationRecorder> create(std::size_t numMolecules, std::size_t snapshotEvery,
                                                             bool isContinuation, Logger &logger) const override;
};


class XYZRecorderFactory : public SimulationRecorderFactory {
private:
    std::string filename;

public:
    explicit XYZRecorderFactory(std::string filename) : filename{std::move(filename)}
    { }

    [[nodiscard]] std::unique_ptr<SimulationRecorder> create(std::size_t numMolecules, std::size_t snapshotEvery,
                                                             bool isContinuation, Logger &logger) const override;
};


#endif //RAMPACK_SIMULATIONRECORDERFACTORY_H
