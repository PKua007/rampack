//
// Created by Piotr Kubala on 16/01/2023.
//

#ifndef RAMPACK_RAMPACKPARAMETERS_H
#define RAMPACK_RAMPACKPARAMETERS_H

#include <array>
#include <utility>
#include <vector>
#include <string>
#include <optional>
#include <variant>

#include "utils/Version.h"
#include "PackingFactory.h"
#include "core/SnapshotWriter.h"
#include "core/SimulationRecorder.h"
#include "core/ObservablesCollector.h"
#include "FileSnapshotWriter.h"
#include "SimulationRecorderFactory.h"


struct BaseParameters {
    Version version;
    std::shared_ptr<PackingFactory> packingFactory;
    Simulation::Environment baseEnvironment;
    std::size_t seed{};
    std::shared_ptr<ShapeTraits> shapeTraits;
    std::array<bool, 3> walls{};
    std::size_t scalingThreads{};
    std::array<std::size_t, 3> domainDivisions{};
    bool saveOnSignal{};
};

struct IntegrationRun {
    std::string runName;
    Simulation::Environment environment;
    std::optional<std::size_t> thermalizationCycles{};
    std::optional<std::size_t> averagingCycles{};
    std::size_t snapshotEvery{};
    std::size_t averagingEvery{};
    std::size_t inlineInfoEvery{};
    std::size_t orientationFixEvery{};
    std::vector<FileSnapshotWriter> lastSnapshotWriters;
    std::optional<std::string> ramsnapOut;
    std::vector<std::shared_ptr<SimulationRecorderFactory>> simulationRecorders;
    std::optional<std::string> ramtrjOut;
    std::shared_ptr<ObservablesCollector> observablesCollector;
    std::optional<std::string> averagesOut;
    std::optional<std::string> observablesOut;
    std::optional<std::string> bulkObservablesOutPattern;
};

struct OverlapRelaxationRun  {
    std::string runName;
    Simulation::Environment environment;
    std::size_t snapshotEvery{};
    std::size_t inlineInfoEvery{};
    std::size_t orientationFixEvery{};
    std::shared_ptr<ShapeTraits> helperShapeTraits;
    std::vector<FileSnapshotWriter> lastSnapshotWriters;
    std::optional<std::string> ramsnapOut;
    std::vector<std::shared_ptr<SimulationRecorderFactory>> simulationRecorders;
    std::optional<std::string> ramtrjOut;
    std::shared_ptr<ObservablesCollector> observablesCollector;
    std::optional<std::string> observablesOut;
};

using Run = std::variant<IntegrationRun, OverlapRelaxationRun>;

struct RampackParameters {
    BaseParameters baseParameters;
    std::vector<Run> runs;
};


inline void combine_environment(Simulation::Environment &env, const Run &run) {
    auto environmentGetter = [](const auto &run) {
        return run.environment;
    };
    auto runEnv = std::visit(environmentGetter, run);
    env.combine(runEnv);
}


#endif //RAMPACK_RAMPACKPARAMETERS_H
