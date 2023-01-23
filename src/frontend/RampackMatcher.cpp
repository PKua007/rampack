//
// Created by Piotr Kubala on 20/01/2023.
//

#include <typeinfo>

#include "RampackMatcher.h"

#include "RampackParameters.h"
#include "ArrangementMatcher.h"
#include "DynamicParameterMatcher.h"
#include "BoxScalerMatcher.h"
#include "ShapeMatcher.h"
#include "MoveSamplerMatcher.h"
#include "ObservablesMatcher.h"
#include "FileSnapshotWriterMatcher.h"
#include "SimulationRecorderFactoryMatcher.h"

#include "core/ObservablesCollector.h"
#include "core/io/RamsnapWriter.h"


using namespace pyon::matcher;

namespace {
    struct BoxScalingDisabled { };

    using ObservableData = ObservablesMatcher::ObservableData;


    MatcherString create_version();
    MatcherArray create_output_last_snapshot();
    MatcherArray create_record_trajectory();
    std::optional<std::string> fetch_ramsnap_out(const std::vector<FileSnapshotWriter> &snapshotWriters);

    std::shared_ptr<ObservablesCollector>
    create_observable_collector(const std::vector<ObservableData> &observables,
                                const std::vector<std::shared_ptr<BulkObservable>> &bulkObservables);

    MatcherDataclass create_integration();
    MatcherDataclass create_overlap_relaxation();
    MatcherAlternative create_run();
    MatcherArray create_runs();
    MatcherAlternative create_move_types();
    MatcherAlternative create_box_scaler();
    MatcherAlternative create_box_move_threads();
    MatcherArray create_domain_divisions();
    Simulation::Environment create_environment(const DataclassData &environment);
    BaseParameters create_base_parameters(const DataclassData &rampack);


    auto runName = MatcherString{}
        .nonEmpty()
        .containsOnlyCharacters([](char c) { return std::isalnum(c) || c == '_'; })
        .describe("with only digits, letters or '_'");

    auto dynamicParameterNone = MatcherNone{}.mapTo([]() -> std::shared_ptr<DynamicParameter> { return nullptr; });
    auto dynamicParameter = dynamicParameterNone | DynamicParameterMatcher::create();

    auto cyclesNone = MatcherNone{}.mapTo<std::optional<std::size_t>>();
    auto cyclesInt = MatcherInt{}
        .positive()
        .mapTo<std::optional<std::size_t>>();
    auto cycles = cyclesInt | cyclesNone;


    MatcherString create_version() {
        return MatcherString{}
            .filter([](const std::string &versionStr) {
                try {
                    Version{versionStr};
                    return true;
                } catch (const PreconditionException &) {
                    return false;
                }
            })
            .describe("with format MAJOR(.MINOR(.PATCH)), eg. 2, 1.1, 3.0.4")
            .filter([](const std::string &versionStr) {
                Version version{versionStr};
                return version >= INPUT_REVAMP_VERSION && version <= CURRENT_VERSION;
            })
            .describe("between " + INPUT_REVAMP_VERSION.str() + " and " + CURRENT_VERSION.str())
            .mapTo([](const std::string &versionStr) {
                return Version(versionStr);
            });
    }

    MatcherArray create_output_last_snapshot() {
        return MatcherArray{}
            .elementsMatch(FileSnapshotWriterMatcher::create())
            .mapToStdVector<FileSnapshotWriter>();
    }

    MatcherArray create_record_trajectory() {
        return MatcherArray{}
            .elementsMatch(SimulationRecorderFactoryMatcher::create())
            .mapToStdVector<std::shared_ptr<SimulationRecorderFactory>>();
    }

    std::optional<std::string> fetch_ramsnap_out(const std::vector<FileSnapshotWriter> &snapshotWriters) {
        for (const auto &fileWriter : snapshotWriters) {
            const auto &writer = fileWriter.getWriter();
            if (typeid(writer) == typeid(RamsnapWriter))
                return fileWriter.getFilename();
        }
        return std::nullopt;
    }

    std::shared_ptr<ObservablesCollector>
    create_observable_collector(const std::vector<ObservableData> &observables,
                                const std::vector<std::shared_ptr<BulkObservable>> &bulkObservables)
    {
        auto collector = std::make_shared<ObservablesCollector>();
        for (const auto &observable : observables)
            collector->addObservable(observable.observable, observable.scope);
        for (const auto &bulkObservable : bulkObservables)
            collector->addBulkObservable(bulkObservable);
        return collector;
    }

    MatcherDataclass create_integration() {
        auto nullableEvery = MatcherInt{}
            .mapTo<std::size_t>();
        auto notNullEvery = nullableEvery;
        notNullEvery.positive();

        auto outString = MatcherString{}
            .nonEmpty()
            .mapTo([](const std::string &out) -> std::optional<std::string> { return out; });
        auto outNone = MatcherNone{}.mapTo<std::optional<std::string>>();
        auto out_ = outString | outNone;

        // TODO: optional snapshot_every
        // TODO: multi-threaded ObservableCollector
        return MatcherDataclass("integration")
            .arguments({{"run_name", runName},
                        {"thermalization_cycles", cycles},
                        {"averaging_cycles", cycles},
                        {"snapshot_every", notNullEvery},
                        {"temperature", dynamicParameter, "None"},
                        {"pressure", dynamicParameter, "None"},
                        {"move_types", create_move_types(), "None"},
                        {"box_move_type", create_box_scaler(), "None"},
                        {"averaging_every", nullableEvery, "0"},
                        {"inline_info_every", notNullEvery, "100"},
                        {"orientation_fix_every", notNullEvery, "10000"},
                        {"output_last_snapshot", create_output_last_snapshot(), "[]"},
                        {"record_trajectory", create_record_trajectory(), "[]"},
                        {"averages_out", out_, "None"},
                        {"observables", ObservablesMatcher::createObservablesMatcher(), "[]"},
                        {"observables_out", out_, "None"},
                        {"bulk_observables", ObservablesMatcher::createBulkObservablesMatcher(), "[]"},
                        {"bulk_observables_out_pattern", out_, "None"}})
            .filter([](const DataclassData &integration) {
                auto thermalizationCycles = integration["thermalization_cycles"].as<std::optional<std::size_t>>();
                auto averagingCycles = integration["averaging_cycles"].as<std::optional<std::size_t>>();
                return thermalizationCycles.has_value() || averagingCycles.has_value();
            })
            .describe("with at least one of: thermalization_cycles, averaging_cycles specified ")
            .filter([](const DataclassData &integration) {
                auto averagingCycles = integration["averaging_cycles"].as<std::optional<std::size_t>>();
                auto averagingEvery = integration["averaging_every"].as<std::size_t>();
                if (!averagingCycles.has_value())
                    return true;
                return averagingEvery > 0;
            })
            .describe("if averaging_cycles is specified, averaging_every > 0 should also be specified")
            .filter([](const DataclassData &integration) {
                auto averagingCycles = integration["averaging_cycles"].as<std::optional<std::size_t>>();
                auto averagesOut = integration["averagesOut"].as<std::optional<std::size_t>>();
                auto bulkObservables
                    = integration["bulk_observables"].as<std::vector<std::shared_ptr<BulkObservable>>>();
                if (averagingCycles.has_value())
                    return true;
                return !averagesOut.has_value() && bulkObservables.empty();
            })
            .describe("if any of: averages_out, bulk_observables is specified, averaging_cycles should also be")
            .filter([](const DataclassData &integration) {
                auto bulkObservables = integration["bulk_observables"].as<std::vector<std::shared_ptr<BulkObservable>>>();
                auto bulkObservablesOutPattern
                    = integration["bulk_observables_out_pattern"].as<std::optional<std::string>>();
                if (bulkObservables.empty())
                    return true;
                return bulkObservablesOutPattern.has_value();
            })
            .describe("if bulk_observables are specified, bulk_observables_out_pattern should also be")
            .mapTo([](const DataclassData &integration) {
                IntegrationRun run;

                run.runName = integration["run_name"].as<std::string>();
                run.environment = create_environment(integration);
                run.thermalizationCycles = integration["thermalization_cycles"].as<std::optional<std::size_t>>();
                run.averagingCycles = integration["averaging_cycles"].as<std::optional<std::size_t>>();
                run.snapshotEvery = integration["snapshot_every"].as<std::size_t>();
                run.averagingEvery = integration["averaging_every"].as<std::size_t>();
                run.inlineInfoEvery = integration["inline_info_every"].as<std::size_t>();
                run.orientationFixEvery = integration["orientation_fix_every"].as<std::size_t>();
                run.lastSnapshotWriters = integration["output_last_snapshot"].as<std::vector<FileSnapshotWriter>>();
                run.ramsnapOut = fetch_ramsnap_out(run.lastSnapshotWriters);
                run.simulationRecorders
                    = integration["record_trajectory"].as<std::vector<std::shared_ptr<SimulationRecorderFactory>>>();

                auto observables = integration["observables"].as<std::vector<ObservableData>>();
                auto bulkObservables
                    = integration["bulk_observables"].as<std::vector<std::shared_ptr<BulkObservable>>>();
                run.observablesCollector
                    = create_observable_collector(observables, bulkObservables);

                run.averagesOut = integration["averages_out"].as<std::optional<std::string>>();
                run.observablesOut = integration["observables_out"].as<std::optional<std::string>>();
                run.bulkObservablesOutPattern
                    = integration["bulk_observables_out_pattern"].as<std::optional<std::string>>();

                return run;
            });
    }

    MatcherDataclass create_overlap_relaxation() {
        return MatcherDataclass("overlap_relaxation");
    }

    MatcherAlternative create_run() {
        return create_integration() | create_overlap_relaxation();
    }

    MatcherArray create_runs() {
        return MatcherArray{}
            .elementsMatch(create_run())
            .nonEmpty()
            .mapToStdVector<Run>();
    }

    MatcherAlternative create_move_types() {
        auto sampler = MoveSamplerMatcher::create();
        auto samplerArray = pyon::matcher::MatcherArray{}
            .elementsMatch(sampler)
            .nonEmpty()
            .mapToStdVector<std::shared_ptr<MoveSampler>>();

        auto samplerNone = MatcherNone{}.mapTo([](){ return std::vector<std::shared_ptr<MoveSampler>>{}; });

        return samplerArray | samplerNone;
    }

    MatcherAlternative create_box_scaler() {
        auto boxScaler = BoxScalerMatcher::create();
        auto disabledScaling = MatcherDataclass("disabled")
            .mapTo([](const DataclassData&) { return BoxScalingDisabled{}; });
        auto boxScalerNone = MatcherNone{}
            .mapTo([]() -> std::shared_ptr<TriclinicBoxScaler> { return nullptr; });

        return boxScaler | disabledScaling | boxScalerNone;
    }

    MatcherAlternative create_box_move_threads() {
        auto moveThreadsMax = MatcherString("max")
            .mapTo([](const std::string&) -> std::size_t { return OMP_MAXTHREADS; });
        auto moveThreadsInt = MatcherInt{}
            .inRange(1, OMP_MAXTHREADS)
            .mapTo<std::size_t>();
        return moveThreadsMax | moveThreadsInt;
    }

    MatcherArray create_domain_divisions() {
        return MatcherArray(MatcherInt{}.positive().mapTo<std::size_t>(), 3)
            .filter([](const ArrayData &array) {
                auto stdArray = array.asStdArray<std::size_t, 3>();
                return std::accumulate(stdArray.begin(), stdArray.end(), 1, std::multiplies<>{}) <= OMP_MAXTHREADS;
            })
            .mapToStdArray<std::size_t, 3>();
    }

    Simulation::Environment create_environment(const DataclassData &environment) {
        auto temperature = environment["temperature"].as<std::shared_ptr<DynamicParameter>>();
        auto pressure = environment["pressure"].as<std::shared_ptr<DynamicParameter>>();
        auto moveSamplers = environment["move_types"].as<std::vector<std::shared_ptr<MoveSampler>>>();
        auto boxScaler = environment["box_move_type"];

        Simulation::Environment env;

        if (temperature != nullptr)
            env.setTemperature(temperature);
        if (pressure != nullptr)
            env.setTemperature(pressure);
        if (!moveSamplers.empty())
            env.setMoveSamplers(std::move(moveSamplers));

        if (boxScaler.is<BoxScalingDisabled>()) {
            env.disableBoxScaling();
        } else {
            auto boxScalerPtr = boxScaler.as<std::shared_ptr<TriclinicBoxScaler>>();
            if (boxScalerPtr != nullptr)
                env.setBoxScaler(boxScalerPtr);
        }

        return env;
    }

    BaseParameters create_base_parameters(const DataclassData &rampack) {
        BaseParameters baseParams;

        baseParams.version = rampack["version"].as<Version>();
        baseParams.packingFactory = rampack["arrangement"].as<std::shared_ptr<PackingFactory>>();
        baseParams.shapeTraits = rampack["shape"].as<std::shared_ptr<ShapeTraits>>();
        baseParams.seed = rampack["seed"].as<std::size_t>();
        baseParams.baseEnvironment = create_environment(rampack);
        baseParams.walls = rampack["walls"].as<std::array<bool, 3>>();
        baseParams.scalingThreads = rampack["box_move_threads"].as<std::size_t>();
        baseParams.domainDivisions = rampack["domain_divisions"].as<std::array<std::size_t, 3>>();
        baseParams.saveOnSignal = rampack["handle_signals"].as<bool>();

        return baseParams;
    }
}


pyon::matcher::MatcherDataclass RampackMatcher::create() {
    auto seed = MatcherInt{}.mapTo<std::size_t>();
    auto walls = MatcherArray(MatcherBoolean{}, 3).mapToStdArray<bool, 3>();

    return pyon::matcher::MatcherDataclass("rampack")
        .arguments({{"version", create_version()},
                    {"arrangement", ArrangementMatcher::create()},
                    {"shape", ShapeMatcher::shape},
                    {"seed", seed},
                    {"runs", create_runs()},
                    {"temperature", dynamicParameter, "None"},
                    {"pressure", dynamicParameter, "None"},
                    {"move_types", create_move_types(), "None"},
                    {"box_move_type", create_box_scaler(), "None"},
                    {"walls", walls, "[False, False, False]"},
                    {"box_move_threads", create_box_move_threads(), "1"},
                    {"domain_divisions", create_domain_divisions(), "[1, 1, 1]"},
                    {"handle_signals", MatcherBoolean{}, "True"}})
        .filter([](const DataclassData &rampack) {
            auto runs = rampack["runs"].as<std::vector<Run>>();
            auto runNameVisitor = [](const Run &run) {
                auto runNameGetter = [](auto &&run) { return run.runName; };
                return std::visit(runNameGetter, run);
            };
            std::vector<std::string> runNames;
            runNames.reserve(runs.size());
            std::transform(runs.begin(), runs.end(), runNames.begin(), runNameVisitor);
            std::sort(runNames.begin(), runNames.end());
            return std::unique(runNames.begin(), runNames.end()) == runNames.end();
        })
        .describe("with unique run names")
        .filter([](const DataclassData &rampack) {
            auto env = rampack["environment"].as<Simulation::Environment>();
            auto firstRun = rampack["runs"].as<std::vector<Run>>().front();
            auto envGetter = [](auto &&run) { return run.environment; };
            auto firstRunEnv = std::visit(envGetter, firstRun);
            env.combine(firstRunEnv);
            return env.isComplete();
        })
        .describe("base environment combined with the first run's environment should be complete")
        .mapTo([](const DataclassData &rampack) {
            RampackParameters params;
            params.baseParameters = create_base_parameters(rampack);
            params.runs = rampack["runs"].as<std::vector<Run>>();
            return params;
        });
}
