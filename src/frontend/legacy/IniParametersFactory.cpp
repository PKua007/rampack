//
// Created by Piotr Kubala on 17/01/2023.
//

#include <utility>
#include <algorithm>
#include <iterator>
#include <fstream>

#include "IniParametersFactory.h"
#include "ShapeFactory.h"
#include "ArrangementFactory.h"
#include "TriclinicBoxScalerFactory.h"
#include "MoveSamplerFactory.h"
#include "ParameterUpdaterFactory.h"
#include "ObservablesCollectorFactory.h"

#include "core/PeriodicBoundaryConditions.h"
#include "core/io/RamsnapWriter.h"
#include "core/io/WolframWriter.h"
#include "core/io/XYZWriter.h"
#include "core/io/RamtrjRecorder.h"
#include "core/io/XYZRecorder.h"

#include "frontend/FileSnapshotWriter.h"

#include "utils/OMPMacros.h"
#include "utils/Utils.h"
#include "utils/ParseUtils.h"


namespace {
    class LegacyPackingFactory : public PackingFactory {
    private:
        std::size_t numOfParticles{};
        std::string boxString{};
        std::string arrangementString{};
        std::shared_ptr<ShapeTraits> shapeTraits{};

    public:
        LegacyPackingFactory(size_t numOfParticles, std::string boxString, std::string arrangementString,
                             const std::shared_ptr<ShapeTraits> &shapeTraits)
                : numOfParticles{numOfParticles}, boxString{std::move(boxString)},
                  arrangementString{std::move(arrangementString)}, shapeTraits{shapeTraits}
        { }

        std::unique_ptr<Packing> createPacking(std::unique_ptr<BoundaryConditions> bc,
                                               [[maybe_unused]] const Interaction &interaction,
                                               std::size_t moveThreads, std::size_t scalingThreads) override
        {
            return legacy::ArrangementFactory::arrangePacking(this->numOfParticles, this->boxString,
                                                              this->arrangementString, std::move(bc),
                                                              this->shapeTraits->getInteraction(),
                                                              shapeTraits->getGeometry(), moveThreads, scalingThreads);
        }
    };

    class RamtrjRecorderFactory : public SimulationRecorderFactory {
    private:
        std::string filename;

    public:
        explicit RamtrjRecorderFactory(std::string filename) : filename{std::move(filename)}
        { }

        [[nodiscard]] std::unique_ptr<SimulationRecorder> create(std::size_t numMolecules, std::size_t snapshotEvery,
                                                                 bool isContinuation, Logger &logger) const override
        {
            std::unique_ptr<std::fstream> inout;

            if (isContinuation) {
                inout = std::make_unique<std::fstream>(
                    this->filename, std::ios_base::in | std::ios_base::out | std::ios_base::binary
                );
            } else {
                inout = std::make_unique<std::fstream>(
                    this->filename,
                    std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc
                );
            }

            ValidateOpenedDesc(*inout, this->filename, "to store RAMTRJ trajectory");
            logger.info() << "RAMTRJ trajectory is stored on the fly to '" << this->filename << "'" << std::endl;
            return std::make_unique<RamtrjRecorder>(std::move(inout), numMolecules, snapshotEvery, isContinuation);
        }
    };

    class XYZRecorderFactory : public SimulationRecorderFactory {
    private:
        std::string filename;

    public:
        explicit XYZRecorderFactory(std::string filename) : filename{std::move(filename)}
        { }

        [[nodiscard]] std::unique_ptr<SimulationRecorder> create([[maybe_unused]] std::size_t numMolecules,
                                                                 [[maybe_unused]] std::size_t snapshotEvery,
                                                                 bool isContinuation, Logger &logger) const override
        {
            std::unique_ptr<std::fstream> inout;

            if (isContinuation)
                inout = std::make_unique<std::fstream>(this->filename, std::ios_base::app);
            else
                inout = std::make_unique<std::fstream>(this->filename, std::ios_base::out);

            ValidateOpenedDesc(*inout, this->filename, "to store XYZ trajectory");
            logger.info() << "XYZ trajectory is stored on the fly to '" << this->filename << "'" << std::endl;
            return std::make_unique<XYZRecorder>(std::move(inout));
        }
    };


    std::vector<std::shared_ptr<MoveSampler>> create_move_samplers(const std::string &moveTypes,
                                                                   const ShapeTraits &traits)
    {
        auto moveSamplerStrings = explode(moveTypes, ',');
        std::vector<std::shared_ptr<MoveSampler>> moveSamplers;
        moveSamplers.reserve(moveSamplerStrings.size());
        for (const auto &moveSamplerString: moveSamplerStrings)
            moveSamplers.push_back(legacy::MoveSamplerFactory::create(moveSamplerString, traits));
        return moveSamplers;
    }

    Simulation::Environment parse_simulation_environment(const InheritableParameters &params, const ShapeTraits &traits)
    {
        Simulation::Environment env;

        if (!params.scalingType.empty()) {
            auto boxScaler = legacy::TriclinicBoxScalerFactory::create(params.scalingType, params.volumeStepSize);
            if (boxScaler == nullptr) {
                env.disableBoxScaling();
            } else {
                Validate(params.volumeStepSize > 0);
                env.setBoxScaler(std::move(boxScaler));
            }
        }

        if (!params.moveTypes.empty())
            env.setMoveSamplers(create_move_samplers(params.moveTypes, traits));

        if (!params.temperature.empty())
            env.setTemperature(legacy::ParameterUpdaterFactory::create(params.temperature));

        if (!params.pressure.empty())
            env.setPressure(legacy::ParameterUpdaterFactory::create(params.pressure));

        return env;
    }

    std::array<std::size_t, 3> parse_domain_divisions(const std::string &domainDivisionsStr) {
        if (domainDivisionsStr.empty())
            return {1, 1, 1};

        std::array<std::size_t, 3> domainDivisions{};
        std::istringstream domainDivisionsStream(domainDivisionsStr);
        domainDivisionsStream >> domainDivisions[0] >> domainDivisions[1] >> domainDivisions[2];
        ValidateMsg(domainDivisionsStream, "Malformed domain divisions, usage: [x divisions] [y ...] [z ...].");

        std::size_t numDomains = std::accumulate(domainDivisions.begin(), domainDivisions.end(), 1, std::multiplies<>{});
        Validate(numDomains > 0);
        Validate(numDomains <= static_cast<std::size_t>(OMP_MAXTHREADS));

        return domainDivisions;
    }

    std::size_t parse_scaling_threads(const std::string &scalingThreadsStr) {
        std::size_t scalingThreads;
        if (scalingThreadsStr.empty())
            scalingThreads = 1;
        else if (scalingThreadsStr == "max")
            scalingThreads = OMP_MAXTHREADS;
        else
            scalingThreads = std::stoul(scalingThreadsStr);
        Validate(scalingThreads > 0);
        Validate(scalingThreads <= static_cast<std::size_t>(OMP_MAXTHREADS));
        return scalingThreads;
    }

    std::array<bool, 3> parse_walls(const std::string &wallsStr) {
        std::array<bool, 3> isWall{};
        for (char c : wallsStr) {
            if (std::isspace(c))
                continue;
            switch (std::tolower(c)) {
                case 'x':   isWall[0] = true;    break;
                case 'y':   isWall[1] = true;    break;
                case 'z':   isWall[2] = true;    break;
                default:    throw ValidationException("unknown wall axis: " + std::string{c});
            }
        }
        return isWall;
    }

    BaseParameters parse_base_parameters(const Parameters &params) {
        BaseParameters baseParams;

        baseParams.version = params.version;
        baseParams.shapeTraits = legacy::ShapeFactory::shapeTraitsFor(params.shapeName, params.shapeAttributes,
                                                                      params.interaction, params.version);
        baseParams.packingFactory = std::make_shared<LegacyPackingFactory>(
            params.numOfParticles, params.initialDimensions, params.initialArrangement, baseParams.shapeTraits
        );
        baseParams.seed = params.seed;
        baseParams.baseEnvironment = parse_simulation_environment(params, *baseParams.shapeTraits);
        baseParams.walls = parse_walls(params.walls);
        baseParams.scalingThreads = parse_scaling_threads(params.scalingThreads);
        baseParams.domainDivisions = parse_domain_divisions(params.domainDivisions);
        baseParams.saveOnSignal = params.saveOnSignal;

        return baseParams;
    }

    std::pair<std::string, std::map<std::string, std::string>>
    parse_filename_and_params(const std::string &str, const std::vector<std::string> &fields)
    {
        std::istringstream in(str);
        std::string filename;
        in >> filename;
        auto params = ParseUtils::parseFields(fields, ParseUtils::tokenize<std::string>(in));
        return {filename, params};
    }

    FileSnapshotWriter parse_wolfram_writer(const std::string &fileParams) {
        auto [filename, params] = parse_filename_and_params(fileParams, {"style", "mesh_divisions"});

        std::string styleStr = "standard";
        if (params.find("style") != params.end()) {
            styleStr = params["style"];
            params.erase("style");
        }

        WolframWriter::WolframStyle wolframStyle{};
        if (styleStr == "standard")
            wolframStyle = WolframWriter::WolframStyle::STANDARD;
        else if (styleStr == "affineTransform")
            wolframStyle = WolframWriter::WolframStyle::AFFINE_TRANSFORM;
        else
            throw ValidationException("Unknown wolfram style: \"" + styleStr + "\"");

        return {filename, "Wolfram", std::make_shared<WolframWriter>(wolframStyle, params)};
    }

    template <typename ConcreteParams>
    std::vector<FileSnapshotWriter> parse_last_snapshot_writers(const ConcreteParams &params) {
        std::vector<FileSnapshotWriter> writers;

        if (!params.packingFilename.empty())
            writers.emplace_back(params.packingFilename, "RAMSNAP", std::make_shared<RamsnapWriter>());
        if (!params.wolframFilename.empty())
            writers.push_back(parse_wolfram_writer(params.wolframFilename));
        if (!params.xyzPackingFilename.empty())
            writers.emplace_back(params.xyzPackingFilename, "XYZ", std::make_shared<XYZWriter>());

        return writers;
    }

    template <typename ConcreteParams>
    std::vector<std::shared_ptr<SimulationRecorderFactory>>
    parse_simulation_recorders(const ConcreteParams &integrationParams)
    {
        std::vector<std::shared_ptr<SimulationRecorderFactory>> factories;

        if (!integrationParams.recordingFilename.empty())
            factories.push_back(std::make_shared<RamtrjRecorderFactory>(integrationParams.recordingFilename));
        if (!integrationParams.xyzRecordingFilename.empty())
            factories.push_back(std::make_shared<XYZRecorderFactory>(integrationParams.xyzRecordingFilename));

        return factories;
    }

    IntegrationRun parse_integration_run(const Parameters::IntegrationParameters &integrationParams,
                                         const BaseParameters &baseParams)
    {
        IntegrationRun run;
        run.runName = integrationParams.runName;
        run.environment = parse_simulation_environment(integrationParams, *baseParams.shapeTraits);
        run.thermalizationCycles = integrationParams.thermalisationCycles;
        run.averagingCycles = integrationParams.averagingCycles;
        run.snapshotEvery = integrationParams.snapshotEvery;
        run.averagingEvery = integrationParams.averagingEvery;
        run.inlineInfoEvery = integrationParams.inlineInfoEvery;
        run.orientationFixEvery = integrationParams.orientationFixEvery;
        run.lastSnapshotWriters = parse_last_snapshot_writers(integrationParams);
        run.simulationRecorders = parse_simulation_recorders(integrationParams);

        run.observablesCollector = legacy::ObservablesCollectorFactory::create(
            explode(integrationParams.observables, ','), explode(integrationParams.bulkObservables, ','),
            baseParams.scalingThreads, baseParams.version
        );

        if (!integrationParams.outputFilename.empty())
            run.averagesOut = integrationParams.outputFilename;
        if (!integrationParams.observableSnapshotFilename.empty())
            run.observablesOut = integrationParams.observableSnapshotFilename;
        if (!integrationParams.bulkObservableFilenamePattern.empty())
            run.bulkObservablesOutPattern = integrationParams.bulkObservableFilenamePattern;

        return run;
    }

    OverlapRelaxationRun
    parse_overlap_relaxation_run(const Parameters::OverlapRelaxationParameters &overlapRelaxationParams,
                                 const BaseParameters &baseParams)
    {
        OverlapRelaxationRun run;

        run.runName = overlapRelaxationParams.runName;
        run.environment = parse_simulation_environment(overlapRelaxationParams, *baseParams.shapeTraits);
        run.snapshotEvery = overlapRelaxationParams.snapshotEvery;
        run.inlineInfoEvery = overlapRelaxationParams.inlineInfoEvery;
        run.orientationFixEvery = overlapRelaxationParams.orientationFixEvery;
        run.lastSnapshotWriters = parse_last_snapshot_writers(overlapRelaxationParams);
        run.simulationRecorders = parse_simulation_recorders(overlapRelaxationParams);

        run.observablesCollector = legacy::ObservablesCollectorFactory::create(
            explode(overlapRelaxationParams.observables, ','), explode(overlapRelaxationParams.bulkObservables, ','),
            baseParams.scalingThreads, baseParams.version
        );

        if (!overlapRelaxationParams.observableSnapshotFilename.empty())
            run.observablesOut = overlapRelaxationParams.observableSnapshotFilename;

        return run;
    }

    Run parse_run(const Parameters::RunParameters &runParams, const BaseParameters &baseParams) {
        if (std::holds_alternative<Parameters::IntegrationParameters>(runParams)) {
            const auto &integrationParams = std::get<Parameters::IntegrationParameters>(runParams);
            return parse_integration_run(integrationParams, baseParams);
        } else if (std::holds_alternative<Parameters::OverlapRelaxationParameters>(runParams)) {
            const auto &overlapRelaxationParams = std::get<Parameters::OverlapRelaxationParameters>(runParams);
            return parse_overlap_relaxation_run(overlapRelaxationParams, baseParams);
        } else {
            throw AssertionException("Unknown run type");
        }
    }

    std::vector<Run> parse_runs(const Parameters &params, const BaseParameters &baseParams) {
        std::vector<Run> runs;
        runs.reserve(params.runsParameters.size());
        std::transform(params.runsParameters.begin(), params.runsParameters.end(), std::back_inserter(runs),
                       [&baseParams](const auto &runParams) { return parse_run(runParams, baseParams); });
        return runs;
    }
}


RampackParameters IniParametersFactory::create(const Parameters &params) {
    Expects(params.version < INPUT_REVAMP_VERSION);

    RampackParameters rampackParams;
    rampackParams.baseParameters = parse_base_parameters(params);
    rampackParams.runs = parse_runs(params, rampackParams.baseParameters);
    return rampackParams;
}
