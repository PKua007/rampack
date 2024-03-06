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


struct IntegrationRun;
struct OverlapRelaxationRun;
struct TransformationRun;
using Run = std::variant<IntegrationRun, OverlapRelaxationRun, TransformationRun>;

class BadParametersCast : public InternalError {
public:
    using InternalError::InternalError;
};

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

    [[nodiscard]] static const RunBase &of(const Run &run);
    [[nodiscard]] static RunBase &of(Run &run);
};

/**
 * @brief Run, in which snapshots are collected.
 */
struct SimulatingRun : public RunBase {
    std::size_t snapshotEvery{};
    std::size_t inlineInfoEvery{};
    std::size_t orientationFixEvery{};
    std::shared_ptr<ObservablesCollector> observablesCollector;
    std::optional<std::string> observablesOut;
    std::vector<std::shared_ptr<SimulationRecorderFactory>> simulationRecorders;
    std::optional<std::string> ramtrjOut;

    [[nodiscard]] static const SimulatingRun &of(const Run &run);
    [[nodiscard]] static SimulatingRun &of(Run &run);
    [[nodiscard]] static bool isInstance(const Run &run);
};

struct IntegrationRun : public SimulatingRun {
    std::optional<std::size_t> thermalizationCycles{};
    std::optional<std::size_t> averagingCycles{};
    std::size_t averagingEvery{};
    std::optional<std::string> averagesOut;
    std::optional<std::string> bulkObservablesOutPattern;

    [[nodiscard]] static const IntegrationRun &of(const Run &run);
    [[nodiscard]] static IntegrationRun &of(Run &run);
    [[nodiscard]] static bool isInstance(const Run &run);
};

struct OverlapRelaxationRun : public SimulatingRun {
    std::shared_ptr<ShapeTraits> helperShapeTraits;

    [[nodiscard]] static const OverlapRelaxationRun &of(const Run &run);
    [[nodiscard]] static OverlapRelaxationRun &of(Run &run);
    [[nodiscard]] static bool isInstance(const Run &run);
};

struct TransformationRun : public RunBase {
    std::vector<std::shared_ptr<LatticeTransformer>> transformers;

    [[nodiscard]] static const TransformationRun &of(const Run &run);
    [[nodiscard]] static TransformationRun &of(Run &run);
    [[nodiscard]] static bool isInstance(const Run &run);
};

struct RampackParameters {
    BaseParameters baseParameters;
    std::vector<Run> runs;
};

void combine_environment(Simulation::Environment &env, const Run &run);


#endif //RAMPACK_RAMPACKPARAMETERS_H
