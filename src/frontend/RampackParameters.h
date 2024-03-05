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
#include "core/lattice/LatticeTransformer.h"
#include "utils/Exceptions.h"
#include "utils/Utils.h"


class UnsupportedParametersSection : public InternalError {
public:
    using InternalError::InternalError;
};

struct IntegrationRun;
struct OverlapRelaxationRun;
struct TransformationRun;
using Run = std::variant<IntegrationRun, OverlapRelaxationRun, TransformationRun>;

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

/**
 * @brief Base class for all runs containing mandatory fields.
 */
struct RunBase {
    std::string runName;
    Simulation::Environment environment;
    std::vector<FileSnapshotWriter> lastSnapshotWriters;
    std::optional<std::string> ramsnapOut;

    static const RunBase &of(const Run &run);
    static RunBase &of(Run &run);
};

/**
 * @brief Run, in which snapshots are collected.
 */
struct SnapshotCollectorRun : public RunBase {
    std::size_t snapshotEvery{};
    std::shared_ptr<ObservablesCollector> observablesCollector;
    std::optional<std::string> observablesOut;
    std::vector<std::shared_ptr<SimulationRecorderFactory>> simulationRecorders;
    std::optional<std::string> ramtrjOut;

    static const SnapshotCollectorRun &of(const Run &run);
    static SnapshotCollectorRun &of(Run &run);
};

struct IntegrationRun : public SnapshotCollectorRun {
    std::optional<std::size_t> thermalizationCycles{};
    std::optional<std::size_t> averagingCycles{};
    std::size_t averagingEvery{};
    std::size_t inlineInfoEvery{};
    std::size_t orientationFixEvery{};
    std::optional<std::string> averagesOut;
    std::optional<std::string> bulkObservablesOutPattern;
};

struct OverlapRelaxationRun : public SnapshotCollectorRun {
    std::size_t inlineInfoEvery{};
    std::size_t orientationFixEvery{};
    std::shared_ptr<ShapeTraits> helperShapeTraits;

};

struct TransformationRun : public RunBase {
    std::vector<std::shared_ptr<LatticeTransformer>> transformers;
};

struct RampackParameters {
    BaseParameters baseParameters;
    std::vector<Run> runs;
};

void combine_environment(Simulation::Environment &env, const Run &run);


#endif //RAMPACK_RAMPACKPARAMETERS_H
