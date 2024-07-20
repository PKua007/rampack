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
protected:
    std::string filename;

public:
    explicit SimulationRecorderFactory(std::string filename) : filename{std::move(filename)}
    { }

    virtual ~SimulationRecorderFactory() = default;

    [[nodiscard]] virtual std::unique_ptr<SimulationRecorder>
    create(const Packing &packing, const ShapeDataManager &manager, std::size_t snapshotEvery, bool isContinuation,
           Logger &logger) const = 0;

    [[nodiscard]] virtual bool createsRamtrj() const = 0;
    [[nodiscard]] const std::string &getFilename() const { return this->filename; }
};


class RamtrjRecorderFactory : public SimulationRecorderFactory {
public:
    explicit RamtrjRecorderFactory(std::string filename) : SimulationRecorderFactory(std::move(filename))
    { }

    [[nodiscard]] std::unique_ptr<SimulationRecorder> create(const Packing &packing, const ShapeDataManager &manager,
                                                             std::size_t snapshotEvery, bool isContinuation,
                                                             Logger &logger) const override;

    [[nodiscard]] bool createsRamtrj() const override { return true; }
};


class XYZRecorderFactory : public SimulationRecorderFactory {
private:
    std::map<std::string, TextualShapeData> textualSpeciesMap;

public:
    explicit XYZRecorderFactory(std::string filename,
                                std::map<std::string, TextualShapeData> textualSpeciesMap = {})
            : SimulationRecorderFactory(std::move(filename)), textualSpeciesMap{std::move(textualSpeciesMap)}
    { }

    [[nodiscard]] std::unique_ptr<SimulationRecorder> create(const Packing &packing, const ShapeDataManager &manager,
                                                             std::size_t snapshotEvery, bool isContinuation,
                                                             Logger &logger) const override;

    [[nodiscard]] bool createsRamtrj() const override { return false; }
};


#endif //RAMPACK_SIMULATIONRECORDERFACTORY_H
