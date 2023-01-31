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
#include "frontend/SimulationRecorderFactory.h"

#include "utils/OMPMacros.h"
#include "utils/Utils.h"
#include "utils/ParseUtils.h"


namespace {
    class LegacyPackingFactory : public PackingFactory {
    private:
        std::size_t numOfParticles{};
        std::string boxString{};
        std::string arrangementString{};

    public:
        LegacyPackingFactory(size_t numOfParticles, std::string boxString, std::string arrangementString)
                : numOfParticles{numOfParticles}, boxString{std::move(boxString)},
                  arrangementString{std::move(arrangementString)}
        { }

        std::unique_ptr<Packing> createPacking(std::unique_ptr<BoundaryConditions> bc, const ShapeTraits &shapeTraits,
                                               std::size_t moveThreads, std::size_t scalingThreads) override
        {
            return legacy::ArrangementFactory::arrangePacking(this->numOfParticles, this->boxString,
                                                              this->arrangementString, std::move(bc),
                                                              shapeTraits, moveThreads, scalingThreads);
        }
    };


    std::vector<std::shared_ptr<MoveSampler>> create_move_samplers(const std::string &moveTypes)
    {
        auto moveSamplerStrings = explode(moveTypes, ',');
        std::vector<std::shared_ptr<MoveSampler>> moveSamplers;
        moveSamplers.reserve(moveSamplerStrings.size());
        for (const auto &moveSamplerString: moveSamplerStrings)
            moveSamplers.push_back(legacy::MoveSamplerFactory::create(moveSamplerString));
        return moveSamplers;
    }

    Simulation::Environment parse_simulation_environment(const InheritableParameters &params)
    {
        Simulation::Environment env;

        if (!params.scalingType.empty()) {
            auto boxScaler = legacy::TriclinicBoxScalerFactory::create(params.scalingType, params.volumeStepSize);
            if (boxScaler == nullptr) {
                env.disableBoxScaling();
            } else {
                ValidateMsg(params.volumeStepSize > 0, "'volumeStepSize' should be positive when scaling is enabled");
                env.setBoxScaler(std::move(boxScaler));
            }
        }

        if (!params.moveTypes.empty())
            env.setMoveSamplers(create_move_samplers(params.moveTypes));

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
        ValidateMsg(numDomains > 0, "Number of domains should be positive");
        ValidateMsg(numDomains <= static_cast<std::size_t>(OMP_MAXTHREADS),
                    "Number of domains cannot be larger than number of available threads ("
                    + std::to_string(OMP_MAXTHREADS) + ")");

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
        ValidateMsg(scalingThreads > 0, "Number of scaling threads should be positive");
        ValidateMsg(scalingThreads <= static_cast<std::size_t>(OMP_MAXTHREADS),
                    "Number of scaling threads cannot be larger than number of available threads ("
                    + std::to_string(OMP_MAXTHREADS) + ")");
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
            params.numOfParticles, params.initialDimensions, params.initialArrangement
        );
        baseParams.seed = params.seed;
        baseParams.baseEnvironment = parse_simulation_environment(params);
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
        run.environment = parse_simulation_environment(integrationParams);
        run.thermalizationCycles = integrationParams.thermalisationCycles;
        run.averagingCycles = integrationParams.averagingCycles;
        run.snapshotEvery = integrationParams.snapshotEvery;
        run.averagingEvery = integrationParams.averagingEvery;
        run.inlineInfoEvery = integrationParams.inlineInfoEvery;
        run.orientationFixEvery = integrationParams.orientationFixEvery;
        run.lastSnapshotWriters = parse_last_snapshot_writers(integrationParams);

        if (!integrationParams.packingFilename.empty())
            run.ramsnapOut = integrationParams.packingFilename;

        run.simulationRecorders = parse_simulation_recorders(integrationParams);

        if (!integrationParams.recordingFilename.empty())
            run.ramtrjOut = integrationParams.recordingFilename;

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
                                 const BaseParameters &baseParams, const std::string &shapeName,
                                 const std::string &shapeAttributes)
    {
        OverlapRelaxationRun run;

        run.runName = overlapRelaxationParams.runName;
        run.environment = parse_simulation_environment(overlapRelaxationParams);
        run.snapshotEvery = overlapRelaxationParams.snapshotEvery;
        run.inlineInfoEvery = overlapRelaxationParams.inlineInfoEvery;
        run.orientationFixEvery = overlapRelaxationParams.orientationFixEvery;
        run.helperShapeTraits = legacy::ShapeFactory::shapeTraitsFor(shapeName, shapeAttributes,
                                                                     overlapRelaxationParams.helperInteraction,
                                                                     baseParams.version);
        run.lastSnapshotWriters = parse_last_snapshot_writers(overlapRelaxationParams);

        if (!overlapRelaxationParams.packingFilename.empty())
            run.ramsnapOut = overlapRelaxationParams.packingFilename;

        run.simulationRecorders = parse_simulation_recorders(overlapRelaxationParams);

        if (!overlapRelaxationParams.recordingFilename.empty())
            run.ramtrjOut = overlapRelaxationParams.recordingFilename;

        run.observablesCollector = legacy::ObservablesCollectorFactory::create(
            explode(overlapRelaxationParams.observables, ','), explode(overlapRelaxationParams.bulkObservables, ','),
            baseParams.scalingThreads, baseParams.version
        );

        if (!overlapRelaxationParams.observableSnapshotFilename.empty())
            run.observablesOut = overlapRelaxationParams.observableSnapshotFilename;

        return run;
    }

    Run parse_run(const Parameters::RunParameters &runParams, const BaseParameters &baseParams,
                  const std::string &shapeName, const std::string &shapeAttributes)
    {
        if (std::holds_alternative<Parameters::IntegrationParameters>(runParams)) {
            const auto &integrationParams = std::get<Parameters::IntegrationParameters>(runParams);
            return parse_integration_run(integrationParams, baseParams);
        } else if (std::holds_alternative<Parameters::OverlapRelaxationParameters>(runParams)) {
            const auto &overlapRelaxationParams = std::get<Parameters::OverlapRelaxationParameters>(runParams);
            return parse_overlap_relaxation_run(overlapRelaxationParams, baseParams, shapeName, shapeAttributes);
        } else {
            AssertThrow("Unknown run type");
        }
    }

    std::vector<Run> parse_runs(const Parameters &params, const BaseParameters &baseParams) {
        std::vector<Run> runs;
        runs.reserve(params.runsParameters.size());
        auto runParser = [&baseParams, &params](const auto &runParams) {
            return parse_run(runParams, baseParams, params.shapeName, params.shapeAttributes);
        };
        std::transform(params.runsParameters.begin(), params.runsParameters.end(), std::back_inserter(runs), runParser);
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
