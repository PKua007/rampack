//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cstdlib>
#include <fstream>
#include <limits>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <set>

#include <cxxopts.hpp>

#include "Frontend.h"
#include "Parameters.h"
#include "utils/Fold.h"
#include "utils/Utils.h"
#include "utils/Assertions.h"
#include "ShapeFactory.h"
#include "ObservablesCollectorFactory.h"
#include "core/Simulation.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/Packing.h"
#include "utils/OMPMacros.h"
#include "core/lattice/DistanceOptimizer.h"
#include "ArrangementFactory.h"
#include "TriclinicBoxScalerFactory.h"
#include "core/shapes/CompoundShapeTraits.h"
#include "MoveSamplerFactory.h"
#include "core/SimulationRecorder.h"
#include "core/SimulationPlayer.h"
#include "PackingLoader.h"
#include "ParameterUpdaterFactory.h"


Parameters Frontend::loadParameters(const std::string &inputFilename) {
    std::ifstream paramsFile(inputFilename);
    ValidateOpenedDesc(paramsFile, inputFilename, "to load input parameters");
    return Parameters(paramsFile);
}

void Frontend::setVerbosityLevel(std::optional<std::string> verbosity, std::optional<std::string> auxOutput,
                                 std::optional<std::string> auxVerbosity)
{
    if (verbosity.has_value())
        this->logger.setVerbosityLevel(this->parseVerbosityLevel(*verbosity));
    else if (auxOutput.has_value())
        this->logger.setVerbosityLevel(Logger::WARN);
    else
        this->logger.setVerbosityLevel(Logger::INFO);

    if (!auxOutput.has_value())
        return;

    this->auxOutStream.open(*auxOutput);
    ValidateOpenedDesc(this->auxOutStream, *auxOutput, "to log messages");

    this->logger.warn() << "Logging output to '" << *auxOutput << "'" << std::endl;

    this->logger.addOutput(this->auxOutStream);

    if (auxVerbosity.has_value())
        this->logger.setVerbosityLevel(this->parseVerbosityLevel(*auxVerbosity), this->auxOutStream);
    else
        this->logger.setVerbosityLevel(Logger::INFO, this->auxOutStream);
}

Logger::LogType Frontend::parseVerbosityLevel(const std::string &verbosityLevelName) const {
    if (verbosityLevelName == "error")
        return Logger::ERROR;
    else if (verbosityLevelName == "warn")
        return Logger::WARN;
    else if (verbosityLevelName == "info")
        return Logger::INFO;
    else if (verbosityLevelName == "verbose")
        return Logger::VERBOSE;
    else if (verbosityLevelName == "debug")
        return Logger::DEBUG;
    else
        die("Unknown verbosity level: " + verbosityLevelName, this->logger);

    return Logger::ERROR;
}

int Frontend::casino(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Monte Carlo sampling for both hard and soft potentials.");

    std::string inputFilename;
    std::string verbosity;
    std::string startFrom;
    std::size_t continuationCycles;
    std::string auxOutput;
    std::string auxVerbosity;

    options.add_options()
            ("h,help", "prints help for this mode")
            ("i,input", "an INI file with parameters. See sample_inputs folder for full parameters documentation",
             cxxopts::value<std::string>(inputFilename))
            ("V,verbosity", "how verbose the output should be. Allowed values, with increasing verbosity: "
                            "error, warn, info, verbose, debug. Defaults to: info if --log-file not specified, "
                            "otherwise to: warn",
             cxxopts::value<std::string>(verbosity))
            ("s,start-from", "when specified, the simulation will be started from the run with the name given. If not "
                             "used in conjunction with --continue option, the packing will be restored from the "
                             "internal representation file of the preceding run. If --continue is used, the current "
                             "run, but finished or aborted in the past, will be loaded instead. When a special value "
                             "'.auto' is specified, auto-detection of starting run will be attempted based on internal "
                             "representation files (all runs in configuration have to output them). If last attempted "
                             "run was unfinished, --continue option without argument is implicitly added",
             cxxopts::value<std::string>(startFrom))
            ("c,continue", "when specified, the thermalization of previously finished or aborted run will be continued "
                           "for as many more cycles as specified. It can be used together with --start-from to specify "
                           "which run should be continued. If the thermalization phase is already over, the error will "
                           "be reported. If 0 is specified (or left blank, since 0 is the default value), "
                           "total number of thermalization cycles from the input file will not be changed",
             cxxopts::value<std::size_t>(continuationCycles)->implicit_value("0"))
            ("l,log-file", "if specified, messages will be logged both on the standard output and to this file. "
                           "Verbosity defaults then to: warn for standard output and to: info for log file, unless "
                           "changed by --verbosity and/or --log-file-verbosity options",
             cxxopts::value<std::string>(auxOutput))
            ("log-file-verbosity", "how verbose the output to the log file should be. Allowed values, with increasing "
                                   "verbosity: error, warn, info, verbose, debug. Defaults to: info",
             cxxopts::value<std::string>(auxVerbosity));

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    std::optional<std::string> verbosityOptional;
    if (parsedOptions.count("verbosity"))
        verbosityOptional = verbosity;
    std::optional<std::string> auxOutputOptional;
    if (parsedOptions.count("log-file"))
        auxOutputOptional = auxOutput;
    std::optional<std::string> auxVerbosityOptional;
    if (parsedOptions.count("log-file-verbosity"))
        auxVerbosityOptional = auxVerbosity;
    this->setVerbosityLevel(verbosityOptional, auxOutputOptional, auxVerbosityOptional);

    // Validate parsed options
    std::string cmd(argv[0]);
    if (argc != 1)
        die("Unexpected positional arguments. See " + cmd + " --help", this->logger);
    if (!parsedOptions.count("input"))
        die("Input file must be specified with option -i [input file name]", this->logger);

    // Load parameters (both from file and inline)
    this->logger.info();
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "General simulation parameters" << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;

    Parameters params = this->loadParameters(inputFilename);
    params.print(this->logger);

    this->logger << "--------------------------------------------------------------------" << std::endl;

    auto shapeTraits = ShapeFactory::shapeTraitsFor(params.shapeName, params.shapeAttributes, params.interaction);

    this->logger << "Interaction centre range : " << shapeTraits->getInteraction().getRangeRadius() << std::endl;
    this->logger << "Total interaction range  : " << shapeTraits->getInteraction().getTotalRangeRadius() << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;

    // Parse number of scaling threads
    std::size_t scalingThreads{};
    if (params.scalingThreads == "max")
        scalingThreads = _OMP_MAXTHREADS;
    else
        scalingThreads = std::stoul(params.scalingThreads);
    Validate(scalingThreads > 0);
    Validate(scalingThreads <= static_cast<std::size_t>(_OMP_MAXTHREADS));

    // Parse domain divisions
    std::array<std::size_t, 3> domainDivisions{};
    std::istringstream domainDivisionsStream(params.domainDivisions);
    domainDivisionsStream >> domainDivisions[0] >> domainDivisions[1] >> domainDivisions[2];
    ValidateMsg(domainDivisionsStream, "Malformed domain divisions, usage: x divisions, y div., z div.");

    std::size_t numDomains = std::accumulate(domainDivisions.begin(), domainDivisions.end(), 1, std::multiplies<>{});
    Validate(numDomains > 0);
    Validate(numDomains <= static_cast<std::size_t>(_OMP_MAXTHREADS));

    // We use the same number of threads for scaling and particle moves, otherwise OpenMP leaks memory
    // Too many domain threads are ok, some will just be jobless. But we cannot use less scaling threads than
    // domain threads
    // See https://stackoverflow.com/questions/67267035/...
    // ...increasing-memory-consumption-for-2-alternating-openmp-parallel-regions-with-dif
    Validate(numDomains <= scalingThreads);

    // Info about threads
    this->logger << _OMP_MAXTHREADS << " OpenMP threads are available" << std::endl;
    this->logger << "Using " << scalingThreads << " threads for scaling moves" << std::endl;
    if (numDomains == 1) {
        this->logger << "Using 1 thread without domain decomposition for particle moves" << std::endl;
    } else {
        this->logger << "Using " << domainDivisions[0] << " x " << domainDivisions[1] << " x ";
        this->logger << domainDivisions[2] << " = " << numDomains << " domains for particle moves" << std::endl;
    }
    this->logger << "--------------------------------------------------------------------" << std::endl;

    // Parse scaling type
    std::unique_ptr<TriclinicBoxScaler> triclinicBoxScaler = TriclinicBoxScalerFactory::create(params.scalingType);

    // Parse move type
    auto moveSamplerStrings = explode(params.moveTypes, ',');
    std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
    moveSamplers.reserve(moveSamplerStrings.size());
    for (const auto &moveSamplerString : moveSamplerStrings)
        moveSamplers.push_back(MoveSamplerFactory::create(moveSamplerString, *shapeTraits));

    // Load starting state from a previous or current run packing depending on --start-from and --continue
    // options combination

    std::optional<std::string> optionalStartFrom;
    std::optional<std::size_t> optionalContinuationCycles;
    if (parsedOptions.count("start-from"))
        optionalStartFrom = startFrom;
    if (parsedOptions.count("continue"))
        optionalContinuationCycles = continuationCycles;

    PackingLoader packingLoader(this->logger, optionalStartFrom, optionalContinuationCycles, params.runsParameters);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    packingLoader.loadPacking(std::move(bc), shapeTraits->getInteraction(), scalingThreads, scalingThreads);
    std::size_t startRunIndex = packingLoader.getStartRunIndex();
    std::size_t cycleOffset = packingLoader.getCycleOffset();
    bool isContinuation = packingLoader.isContinuation();

    if (packingLoader.isAllFinished()) {
        this->logger.warn() << "No runs left to be performed. Exiting." << std::endl;
        return EXIT_SUCCESS;
    }

    std::unique_ptr<Packing> packing;
    if (packingLoader.isRestored()) {
        packing = packingLoader.releasePacking();
        const auto &auxInfo = packingLoader.getAuxInfo();
        this->overwriteMoveStepSizes(moveSamplers, auxInfo);
        std::string scalingKey = Frontend::formatMoveKey("scaling", "scaling");
        params.volumeStepSize = std::stod(auxInfo.at(scalingKey));
        Validate(params.volumeStepSize > 0);
    } else {
        // Same number of scaling and domain threads
        bc = std::make_unique<PeriodicBoundaryConditions>();
        packing = ArrangementFactory::arrangePacking(params.numOfParticles, params.initialDimensions,
                                                     params.initialArrangement, std::move(bc),
                                                     shapeTraits->getInteraction(), shapeTraits->getGeometry(),
                                                     scalingThreads, scalingThreads);
    }

    this->createWalls(*packing, params.walls);

    // Perform simulations starting from initial run
    Simulation simulation(std::move(packing), std::move(moveSamplers), params.volumeStepSize, params.seed,
                          std::move(triclinicBoxScaler), domainDivisions, params.saveOnSignal);

    for (std::size_t i = startRunIndex; i < params.runsParameters.size(); i++) {
        if (std::holds_alternative<Parameters::IntegrationParameters>(params.runsParameters[i])) {
            const auto &runParams = std::get<Parameters::IntegrationParameters>(params.runsParameters[i]);
            this->performIntegration(simulation, runParams, *shapeTraits, cycleOffset, isContinuation);
        } else if (std::holds_alternative<Parameters::OverlapRelaxationParameters>(params.runsParameters[i])) {
            const auto &runParams = std::get<Parameters::OverlapRelaxationParameters>(params.runsParameters[i]);
            this->performOverlapRelaxation(simulation, params.shapeName, params.shapeAttributes, runParams, shapeTraits,
                                           cycleOffset, isContinuation);
        } else {
            throw AssertionException("Unimplemented run type");
        }

        isContinuation = false;
        cycleOffset = 0;
        
        if (simulation.wasInterrupted())
            break;
    }

    return EXIT_SUCCESS;
}

void Frontend::performIntegration(Simulation &simulation, const Parameters::IntegrationParameters &runParams,
                                  const ShapeTraits &shapeTraits, size_t cycleOffset, bool isContinuation)
{
    this->logger.setAdditionalText(runParams.runName);
    this->logger.info() << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "Starting integration '" << runParams.runName << "'" << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;
    runParams.print(this->logger);
    this->logger << "--------------------------------------------------------------------" << std::endl;

    std::unique_ptr<SimulationRecorder> recorder;
    if (!runParams.recordingFilename.empty()) {
        recorder = this->loadSimulationRecorder(runParams.recordingFilename, simulation.getPacking().size(),
                                                runParams.snapshotEvery, isContinuation);
    }

    auto temperatureUpdater = ParameterUpdaterFactory::create(runParams.temperature);
    auto pressureUpdater = ParameterUpdaterFactory::create(runParams.pressure);
    auto collector = ObservablesCollectorFactory::create(explode(runParams.observables, ','),
                                                         explode(runParams.bulkObservables, ','),
                                                         simulation.getPacking().getScalingThreads());
    simulation.integrate(std::move(temperatureUpdater), std::move(pressureUpdater), runParams.thermalisationCycles,
                         runParams.averagingCycles, runParams.averagingEvery, runParams.snapshotEvery,
                         shapeTraits, std::move(collector), std::move(recorder), logger, cycleOffset);
    const ObservablesCollector &observablesCollector = simulation.getObservablesCollector();

    this->logger.info();
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->printAverageValues(observablesCollector);
    this->printPerformanceInfo(simulation);

    if (!runParams.packingFilename.empty())
        this->storePacking(simulation, runParams.packingFilename);
    if (!runParams.wolframFilename.empty())
        this->storeWolframVisualization(simulation.getPacking(), shapeTraits.getPrinter(), runParams.wolframFilename);
    if (!runParams.outputFilename.empty()) {
        this->storeAverageValues(runParams.outputFilename, observablesCollector, simulation.getCurrentTemperature(),
                                 simulation.getCurrentPressure());
    }
    if (!runParams.observableSnapshotFilename.empty())
        this->storeSnapshots(observablesCollector, isContinuation, runParams.observableSnapshotFilename);
    if (!runParams.bulkObservableFilenamePattern.empty())
        this->storeBulkObservables(observablesCollector, runParams.bulkObservableFilenamePattern);
}

std::unique_ptr<SimulationRecorder> Frontend::loadSimulationRecorder(const std::string &filename,
                                                                     std::size_t numMolecules, std::size_t cycleStep,
                                                                     bool &isContinuation) const {
    std::unique_ptr<std::fstream> inout;

    if (isContinuation) {
        inout = std::make_unique<std::fstream>(
            filename, std::ios_base::in | std::ios_base::out | std::ios_base::binary
        );
    } else {
        inout = std::make_unique<std::fstream>(
            filename, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc
        );
    }

    ValidateOpenedDesc(*inout, filename, "to store packing data");
    return std::make_unique<SimulationRecorder>(std::move(inout), numMolecules, cycleStep, isContinuation);
}

void Frontend::performOverlapRelaxation(Simulation &simulation, const std::string &shapeName,
                                        const std::string &shapeAttr,
                                        const Parameters::OverlapRelaxationParameters &runParams,
                                        std::shared_ptr<ShapeTraits> shapeTraits, size_t cycleOffset,
                                        bool isContinuation)
{
    this->logger.setAdditionalText(runParams.runName);
    this->logger.info() << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "Starting overlap relaxation '" << runParams.runName << "'" << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;
    runParams.print(this->logger);
    this->logger << "--------------------------------------------------------------------" << std::endl;

    std::unique_ptr<SimulationRecorder> recorder;
    if (!runParams.recordingFilename.empty()) {
        recorder = this->loadSimulationRecorder(runParams.recordingFilename, simulation.getPacking().size(),
                                                runParams.snapshotEvery, isContinuation);
    }

    if (!runParams.helperInteraction.empty()) {
        auto helperShape = ShapeFactory::shapeTraitsFor(shapeName, shapeAttr, runParams.helperInteraction);
        shapeTraits = std::make_shared<CompoundShapeTraits>(shapeTraits, helperShape);
    }

    auto temperatureUpdater = ParameterUpdaterFactory::create(runParams.temperature);
    auto pressureUpdater = ParameterUpdaterFactory::create(runParams.pressure);
    auto collector = ObservablesCollectorFactory::create(explode(runParams.observables, ','),
                                                         explode(runParams.bulkObservables, ','),
                                                         simulation.getPacking().getScalingThreads());
    simulation.relaxOverlaps(std::move(temperatureUpdater), std::move(pressureUpdater), runParams.snapshotEvery,
                             *shapeTraits, std::move(collector), std::move(recorder), this->logger, cycleOffset);
    const ObservablesCollector &observablesCollector = simulation.getObservablesCollector();

    this->logger.info();
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->printPerformanceInfo(simulation);

    if (!runParams.packingFilename.empty())
        this->storePacking(simulation, runParams.packingFilename);
    if (!runParams.wolframFilename.empty())
        this->storeWolframVisualization(simulation.getPacking(), shapeTraits->getPrinter(), runParams.wolframFilename);
    if (!runParams.observableSnapshotFilename.empty())
        this->storeSnapshots(observablesCollector, isContinuation, runParams.observableSnapshotFilename);
    if (!runParams.bulkObservableFilenamePattern.empty())
        this->storeBulkObservables(observablesCollector, runParams.bulkObservableFilenamePattern);
}

void Frontend::storeSnapshots(const ObservablesCollector &observablesCollector, bool isContinuation,
                              const std::string &observableSnapshotFilename) const
{
    if (isContinuation) {
        std::ofstream out(observableSnapshotFilename, std::ios_base::app);
        ValidateOpenedDesc(out, observableSnapshotFilename, "to store observables");
        observablesCollector.printSnapshots(out, false);
    } else {
        std::ofstream out(observableSnapshotFilename);
        ValidateOpenedDesc(out, observableSnapshotFilename, "to store observables");
        observablesCollector.printSnapshots(out, true);
    }

    this->logger.info() << "Observable snapshots stored to " + observableSnapshotFilename << std::endl;
}

void Frontend::storeWolframVisualization(const Packing &packing, const ShapePrinter &shapePrinter,
                                         const std::string &wolframAttr) const
{
    std::istringstream wolframAttrStream(wolframAttr);
    std::string filename;
    std::string styleStr;
    wolframAttrStream >> filename >> styleStr;
    if (!wolframAttrStream)
        styleStr = "standard";

    Packing::WolframStyle wolframStyle{};
    if (styleStr == "standard")
        wolframStyle = Packing::WolframStyle::STANDARD;
    else if (styleStr == "affineTransform")
        wolframStyle = Packing::WolframStyle::AFFINE_TRANSFORM;
    else
        throw ValidationException("Unknown Packing::toWolfram style: " + styleStr);

    std::ofstream out(filename);
    ValidateOpenedDesc(out, filename, "to store Wolfram packing");
    packing.toWolfram(out, shapePrinter, wolframStyle);
    this->logger.info() << "Wolfram packing stored to " + filename << " using '" << styleStr << "' style" << std::endl;
}

void Frontend::storePacking(const Simulation &simulation, const std::string &packingFilename) {
    std::map<std::string, std::string> auxInfo;

    auxInfo["cycles"] = std::to_string(simulation.getTotalCycles());

    const auto &movesStatistics = simulation.getMovesStatistics();
    for (const auto &moveStatistics : movesStatistics) {
        auto groupName = moveStatistics.groupName;
        for (const auto &stepSizeData : moveStatistics.stepSizeDatas) {
            std::string moveKey = Frontend::formatMoveKey(groupName, stepSizeData.moveName);
            auxInfo[moveKey] = doubleToString(stepSizeData.stepSize);
        }
    }

    std::ofstream out(packingFilename);
    ValidateOpenedDesc(out, packingFilename, "to store packing data");
    simulation.getPacking().store(out, auxInfo);

    this->logger.info() << "Packing stored to " + packingFilename << std::endl;
}

void Frontend::printPerformanceInfo(const Simulation &simulation) {
    const auto &simulatedPacking = simulation.getPacking();
    std::size_t ngRebuilds = simulatedPacking.getNeighbourGridRebuilds();
    std::size_t ngResizes = simulatedPacking.getNeighbourGridResizes();

    double totalSeconds = simulation.getTotalMicroseconds() / 1e6;
    double ngRebuildSeconds = simulatedPacking.getNeighbourGridRebuildMicroseconds() / 1e6;
    double moveSeconds = simulation.getMoveMicroseconds() / 1e6;
    double scalingSeconds = simulation.getScalingMicroseconds() / 1e6;
    double domainDecompositionSeconds = simulation.getDomainDecompositionMicroseconds() / 1e6;
    double observablesSeconds = simulation.getObservablesMicroseconds() / 1e6;
    double otherSeconds = totalSeconds - moveSeconds - scalingSeconds - observablesSeconds;
    double cyclesPerSecond = static_cast<double>(simulation.getPerformedCycles()) / totalSeconds;

    double ngRebuildTotalPercent = ngRebuildSeconds / totalSeconds * 100;
    double ngRebuildScalingPercent = ngRebuildSeconds / scalingSeconds * 100;
    double domainDecompTotalPercent = domainDecompositionSeconds / totalSeconds * 100;
    double domainDecompMovePercent = domainDecompositionSeconds / moveSeconds * 100;
    double movePercent = moveSeconds / totalSeconds * 100;
    double scalingPercent = scalingSeconds / totalSeconds * 100;
    double observablesPercent = observablesSeconds / totalSeconds * 100;
    double otherPercent = otherSeconds / totalSeconds * 100;

    this->printMoveStatistics(simulation);
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "Neighbour grid resizes/rebuilds : " << ngResizes << "/" << ngRebuilds << std::endl;
    this->logger << "Average neighbours per centre   : " << simulatedPacking.getAverageNumberOfNeighbours();
    this->logger << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "Cycles per second   : " << cyclesPerSecond << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "Total time          : " << std::right << std::setw(11) << totalSeconds << " s" << std::endl;
    this->logger << "Move time           : " << std::right << std::setw(11) << moveSeconds << " s (" << movePercent << "% total)" << std::endl;
    this->logger << "Dom. decomp. time   : " << std::right << std::setw(11) << domainDecompositionSeconds << " s (";
    this->logger << domainDecompMovePercent << "% move, " << domainDecompTotalPercent << "% total)" << std::endl;
    this->logger << "Scaling time        : " << std::right << std::setw(11) << scalingSeconds << " s (" << scalingPercent << "% total)" << std::endl;
    this->logger << "NG rebuild time     : " << std::right << std::setw(11) << ngRebuildSeconds << " s (";
    this->logger << ngRebuildScalingPercent << "% scaling, " << ngRebuildTotalPercent << "% total)" << std::endl;
    this->logger << "Observables time    : " << std::right << std::setw(11) << observablesSeconds << " s (";
    this->logger << observablesPercent << "% total)" << std::endl;
    this->logger << "Other time          : " << std::right << std::setw(11) << otherSeconds << " s (" << otherPercent << "% total)" << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;
}

int Frontend::printGeneralHelp(const std::string &cmd) {
    std::ostream &rawOut = this->logger;

    rawOut << Fold("Random and Maximal PACKing PACKage - computational package dedicated to simulate various packing "
                   "models (currently only Monte Carlo is available).").width(80) << std::endl;
    rawOut << std::endl;
    rawOut << "Usage: " << cmd << " [mode] (mode dependent parameters). " << std::endl;
    rawOut << std::endl;
    rawOut << "Available modules:" << std::endl;
    rawOut << "casino" << std::endl;
    rawOut << Fold("Monte Carlo sampling for both hard and soft potentials.").width(80).margin(4) << std::endl;
    rawOut << "optimize-distance" << std::endl;
    rawOut << Fold("Find minimal distances between shapes in given direction(s).")
              .width(80).margin(4) << std::endl;
    rawOut << "preview" << std::endl;
    rawOut << Fold("Based on the input file generate initial configuration and store Wolfram and/or *.dat packing.")
              .width(80).margin(4) << std::endl;
    rawOut << "trajectory" << std::endl;
    rawOut << Fold("Replays recorded simulation trajectory and performs some operations on it.")
              .width(80).margin(4) << std::endl;
    rawOut << "shape-preview" << std::endl;
    rawOut << Fold("Provides information and preview for a given shape.").width(80).margin(4) << std::endl;
    rawOut << std::endl;
    rawOut << "Type " + cmd + " [mode] --help to get help on the specific mode." << std::endl;

    return EXIT_SUCCESS;
}

void Frontend::printAverageValues(const ObservablesCollector &collector) {
    auto groupedAverageValues = collector.getGroupedAverageValues();

    auto lengthComparator = [](const auto &desc1, const auto &desc2) {
        return desc1.groupName.length() < desc2.groupName.length();
    };
    std::size_t maxLength = std::max_element(groupedAverageValues.begin(), groupedAverageValues.end(),
                                             lengthComparator)->groupName.length();

    for (auto &observableGroup : groupedAverageValues) {
        this->logger << "Average " << std::left << std::setw(maxLength);
        this->logger << observableGroup.groupName << " : ";
        Assert(!observableGroup.observableData.empty());
        for (std::size_t i{}; i < observableGroup.observableData.size() - 1; i++) {
            auto &data = observableGroup.observableData[i];
            data.quantity.separator = Quantity::PLUS_MINUS;
            this->logger << data.name << " = " << data.quantity << ", ";
        }
        auto &data = observableGroup.observableData.back();
        data.quantity.separator = Quantity::PLUS_MINUS;
        this->logger << data.name << " = " << data.quantity << std::endl;
    }

    this->logger << "--------------------------------------------------------------------" << std::endl;
}

void Frontend::storeAverageValues(const std::string &filename, const ObservablesCollector &collector,
                                  double temperature, double pressure) const
{
    auto flatValues = collector.getFlattenedAverageValues();

    std::ofstream out;
    if (!std::filesystem::exists(filename)) {
        out.open(filename);
        ValidateOpenedDesc(out, filename, "to store average values");
        out << "temperature pressure ";
        for (const auto &value : flatValues)
            out << value.name << " d" << value.name << " ";
        out << std::endl;
    } else {
        out.open(filename, std::ios_base::app);
        ValidateOpenedDesc(out, filename, "to store average values");
    }

    out.precision(std::numeric_limits<double>::max_digits10);
    out << temperature << " " << pressure << " ";
    for (auto &value : flatValues) {
        value.quantity.separator = Quantity::SPACE;
        out << value.quantity << " ";
    }
    out << std::endl;

    this->logger.info() << "Average values stored to " + filename << std::endl;
}

int Frontend::optimize_distance(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Tangent distance optimizer.");

    std::string inputFilename;
    std::string shapeName;
    std::string shapeAttributes;
    std::string interaction;
    std::string rotation1Str;
    std::string rotation2Str;
    std::vector<std::string> directionsStr;
    bool minimalOutput = false;

    options.add_options()
        ("h,help", "prints help for this mode")
        ("i,input", "loads shape parameters from INI file with parameters. If not specified, --shape-name must be "
                    "specified manually",
         cxxopts::value<std::string>(inputFilename))
        ("s,shape-name", "if specified, overrides shape name from --input", cxxopts::value<std::string>(shapeName))
        ("a,shape-attributes", "if specified, overrides shape attributes from --input. If not specified and no "
                               "--input is passed, it defaults to an empty string",
         cxxopts::value<std::string>(shapeAttributes))
        ("I,interaction", "if specified, overrides interaction from --input. If not specified and and no --input is "
                          "passed, it defaults to the empty string",
         cxxopts::value<std::string>(interaction))
        ("1,rotation-1", "[x angle] [y angle] [z angle] - the external Euler angles in degrees to rotate the 1st shape",
         cxxopts::value<std::string>(rotation1Str)->default_value("0 0 0"))
        ("2,rotation-2", "[x angle] [y angle] [z angle] - the external Euler angles in degrees to rotate the 2nd shape",
         cxxopts::value<std::string>(rotation2Str)->default_value("0 0 0"))
        ("d,direction", "[x] [y] [z] - if specified, the minimal distance will be computed in the direction given by "
                        "3D vector with its coordinates as specified. The option may be used more than once",
         cxxopts::value<std::vector<std::string>>(directionsStr))
        ("A,axes", "if specified, the distance will be computed for x, y and z axes")
        ("m,minimal-output", "output only distances - easier to parse in automated workflows");

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    if (parsedOptions.count("minimal-output")) {
        minimalOutput = true;
        this->logger.setVerbosityLevel(Logger::ERROR);
    }

    // Validate parsed options
    std::string cmd(argv[0]);
    if (argc != 1)
        die("Unexpected positional arguments. See " + cmd + " --help", this->logger);
    if (!parsedOptions.count("input") && !parsedOptions.count("shape-name"))
        die("You must specify --input file or --shape-name", this->logger);
    if (!parsedOptions.count("direction") && !parsedOptions.count("axes"))
        die("You must specify at least one --direction or use --axes", this->logger);

    // Load parameters from file if specified
    if (parsedOptions.count("input")) {
        Parameters params = this->loadParameters(inputFilename);
        this->logger.info() << "Loaded shape parameters from '" << inputFilename << "'" << std::endl;
        if (!parsedOptions.count("shape-name"))
            shapeName = params.shapeName;
        if (!parsedOptions.count("shape-attributes"))
            shapeAttributes = params.shapeAttributes;
        if (!parsedOptions.count("interaction"))
            interaction = params.interaction;
    }

    // Axes option - add x, y, z axes to directions
    if (parsedOptions.count("axes")) {
        directionsStr.emplace_back("1 0 0");
        directionsStr.emplace_back("0 1 0");
        directionsStr.emplace_back("0 0 1");
    }

    // Parse directions
    std::vector<Vector<3>> directions;
    directions.reserve(directionsStr.size());
    auto directionParser = [](const std::string &directionStr) {
        Vector<3> direction;
        std::istringstream directionStream(directionStr);
        directionStream >> direction[0] >> direction[1] >> direction[2];
        ValidateMsg(directionStream, "Malformed direction '" + directionStr + "'. Expected format: [x] [y] [z]");
        Validate(direction.norm2() > 1e-12);
        return direction;
    };
    std::transform(directionsStr.begin(), directionsStr.end(), std::back_inserter(directions), directionParser);

    // Parse rotations
    auto rotationParser = [](const std::string &rotationStr) {
        double angleX, angleY, angleZ;
        std::istringstream rotationStream(rotationStr);
        rotationStream >> angleX >> angleY >> angleZ;
        ValidateMsg(rotationStream, "Malformed rotation '" + rotationStr + "'. Expected format: "
                                    + "[angle x] [angle y] [angle z]");
        double factor = M_PI/180;
        return Matrix<3, 3>::rotation(angleX*factor, angleY*factor, angleZ*factor);
    };
    Matrix<3, 3> rotation1 = rotationParser(rotation1Str);
    Matrix<3, 3> rotation2 = rotationParser(rotation2Str);
    Shape shape1, shape2;
    shape1.setOrientation(rotation1);
    shape2.setOrientation(rotation2);

    auto shapeTraits = ShapeFactory::shapeTraitsFor(shapeName, shapeAttributes, interaction);

    this->logger.info() << "Shape name       : " << shapeName << std::endl;
    this->logger.info() << "Shape attributes : " << shapeAttributes << std::endl;
    this->logger.info() << "Interaction      : " << interaction<< std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;

    for (const auto &direction : directions) {
        std::ostringstream minimalDistanceStream;
        minimalDistanceStream.precision(std::numeric_limits<double>::max_digits10);
        minimalDistanceStream << DistanceOptimizer::minimizeForDirection(shape1, shape2, direction,
                                                                         shapeTraits->getInteraction());
        if (minimalOutput)
            this->logger.raw() << minimalDistanceStream.str() << std::endl;
        else
            this->logger << direction << ": " << minimalDistanceStream.str() << std::endl;
    }

    return EXIT_SUCCESS;
}

int Frontend::preview(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Initial arrangement preview.");

    std::string inputFilename;
    std::string datFilename;
    std::string wolframFilename;

    options.add_options()
        ("h,help", "prints help for this mode")
        ("i,input", "an INI file with parameters. See sample_inputs folder for full parameters documentation",
         cxxopts::value<std::string>(inputFilename))
        ("w,wolfram", "if specified, Mathematica notebook with the packing will be generated",
         cxxopts::value<std::string>(wolframFilename))
        ("d,dat", "if specified, *.dat file with packing will be generated",
         cxxopts::value<std::string>(datFilename));

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    // Validate parsed options
    std::string cmd(argv[0]);
    if (argc != 1)
        die("Unexpected positional arguments. See " + cmd + " --help", this->logger);
    if (!parsedOptions.count("input"))
        die("Input file must be specified with option -i [input file name]", this->logger);
    if (!parsedOptions.count("wolfram") && !parsedOptions.count("dat"))
        die("At least one of: --wolfram, --dat options must be specified", this->logger);

    Parameters params = this->loadParameters(inputFilename);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    auto shapeTraits = ShapeFactory::shapeTraitsFor(params.shapeName, params.shapeAttributes, params.interaction);
    auto packing = ArrangementFactory::arrangePacking(params.numOfParticles, params.initialDimensions,
                                                      params.initialArrangement, std::move(bc),
                                                      shapeTraits->getInteraction(), shapeTraits->getGeometry(), 1, 1);
    this->createWalls(*packing, params.walls);

    // Store packing (if desired)
    if (parsedOptions.count("dat"))
        this->generateDatFile(*packing, params, *shapeTraits, datFilename);

    // Store Mathematica packing (if desired)
    if (parsedOptions.count("wolfram"))
        this->storeWolframVisualization(*packing, shapeTraits->getPrinter(), wolframFilename);

    return EXIT_SUCCESS;
}

void Frontend::generateDatFile(const Packing &packing, const Parameters &params, const ShapeTraits &traits,
                               const std::string &datFilename, std::size_t cycles)
{
    // Parse move type
    auto moveSamplerStrings = explode(params.moveTypes, ',');
    std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
    moveSamplers.reserve(moveSamplerStrings.size());
    for (const auto &moveSamplerString : moveSamplerStrings)
        moveSamplers.push_back(MoveSamplerFactory::create(moveSamplerString, traits));

    std::map<std::string, std::string> auxInfo;
    appendMoveStepSizesToAuxInfo(moveSamplers, params.volumeStepSize, auxInfo);
    auxInfo["cycles"] = std::to_string(cycles);

    std::ofstream out(datFilename);
    ValidateOpenedDesc(out, datFilename, "to store packing data");
    packing.store(out, auxInfo);
    this->logger.info() << "Packing stored to " + datFilename << std::endl;
}

std::string Frontend::doubleToString(double d) {
    std::ostringstream ostr;
    ostr.precision(std::numeric_limits<double>::max_digits10);
    ostr << d;
    return ostr.str();
}

void Frontend::printMoveStatistics(const Simulation &simulation) const {
    auto movesStatistics = simulation.getMovesStatistics();

    for (const auto &moveStatistics : movesStatistics) {
        std::string groupName = moveStatistics.groupName;
        groupName.front() = static_cast<char>(std::toupper(groupName.front()));
        this->logger << groupName << " move statistics:" << std::endl;
        this->logger << "  Rate       : " << moveStatistics.getRate() << " (" << moveStatistics.acceptedMoves << "/";
        this->logger << moveStatistics.totalMoves << ")" << std::endl;
        this->logger << "  Step sizes : ";

        const auto &stepSizeDatas = moveStatistics.stepSizeDatas;
        for (std::size_t i{}; i < stepSizeDatas.size(); i++) {
            const auto &stepSizeData = stepSizeDatas[i];
            this->logger << stepSizeData.moveName << ": " << stepSizeData.stepSize;
            if (i < stepSizeDatas.size() - 1)
                this->logger << ", ";
        }

        this->logger << std::endl;
    }
}

void Frontend::overwriteMoveStepSizes(const std::vector<std::unique_ptr<MoveSampler>> &moveSamplers,
                                      const std::map<std::string, std::string> &packingAuxInfo) const
{
    std::set<std::string> notUsedStepSizes;
    std::string scalingKey = Frontend::formatMoveKey("scaling", "scaling");
    for (const auto &[key, value] : packingAuxInfo) {
        if (Frontend::isStepSizeKey(key) && key != scalingKey)
            notUsedStepSizes.insert(key);
    }

    for (auto &moveSampler : moveSamplers) {
        auto groupName = moveSampler->getName();
        for (const auto &[moveName, stepSize] : moveSampler->getStepSizes()) {
            std::string moveKey = Frontend::formatMoveKey(groupName, moveName);
            if (packingAuxInfo.find(moveKey) == packingAuxInfo.end()) {
                this->logger.warn() << "Step size " << moveKey << " not found in *.dat metadata. Falling back to ";
                this->logger << "input file value " << stepSize << std::endl;
                continue;
            }

            notUsedStepSizes.erase(moveKey);
            moveSampler->setStepSize(moveName, std::stod(packingAuxInfo.at(moveKey)));
        }
    }

    if (notUsedStepSizes.empty())
        return;

    this->logger.warn() << "Packing *.dat file contained metadata for unused move step sizes:" << std::endl;
    for (const auto &notUsedStepSize : notUsedStepSizes)
        this->logger << notUsedStepSize << " = " << packingAuxInfo.at(notUsedStepSize) << std::endl;
}

bool Frontend::isStepSizeKey(const std::string &key) {
    return startsWith(key, "step.");
}

std::string Frontend::formatMoveKey(const std::string &groupName, const std::string &moveName) {
    std::string moveKey = "step.";
    moveKey += groupName;
    moveKey += ".";
    moveKey += moveName;
    return moveKey;
}

void Frontend::appendMoveStepSizesToAuxInfo(const std::vector<std::unique_ptr<MoveSampler>> &moveSamplers,
                                            double scalingStepSize, std::map<std::string, std::string> &auxInfo) const
{
    for (const auto &moveSampler : moveSamplers) {
        auto groupName = moveSampler->getName();
        for (auto [moveName, stepSize] : moveSampler->getStepSizes()) {
            std::string moveKey = Frontend::formatMoveKey(groupName, moveName);
            auxInfo[moveKey] = Frontend::doubleToString(stepSize);
        }
    }
    std::string scalingKey = Frontend::formatMoveKey("scaling", "scaling");
    auxInfo[scalingKey] = Frontend::doubleToString(scalingStepSize);
}

int Frontend::trajectory(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Recorded trajectories analyzer.");

    std::string inputFilename;
    std::string trajectoryFilename;
    std::string obsOutputFilename;
    std::vector<std::string> observables;
    std::string bulkObsOutputFilename;
    std::vector<std::string> bulkObservables;
    std::size_t averagingStart{};
    std::size_t maxThreads{};
    std::string datFilename;
    std::string wolframFilename;
    std::string verbosity;
    std::string auxOutput;
    std::string auxVerbosity;
    std::string storeFilename;

    options.add_options()
        ("h,help", "prints help for this mode")
        ("i,input", "an INI file with parameters that was used to generate the trajectories. See sample_inputs "
                    "folder for full parameters documentation",
         cxxopts::value<std::string>(inputFilename))
        ("t,trajectory", "a file with recorder trajectory",
         cxxopts::value<std::string>(trajectoryFilename))
        ("f,auto-fix", "tries to auto-fix the trajectory if it is broken")
        ("V,verbosity", "how verbose the output should be. Allowed values, with increasing verbosity: "
                        "error, warn, info, verbose, debug. Defaults to: info if --log-file not specified, "
                        "otherwise to: warn",
         cxxopts::value<std::string>(verbosity))
        ("o,obs-output", "output file with the results of --observables",
         cxxopts::value<std::string>(obsOutputFilename))
        ("O,observables", "replays the simulation and calculates specified observables (format as in the input file). "
                          "Observables can be passed using multiple short options (-o obs1 -o obs2) or comma-separated "
                          "in a long option (--observables=obs1,obs2)",
         cxxopts::value<std::vector<std::string>>(observables))
        ("b,bulk-obs-output", "output file pattern with the results of --bulk-observables. Every occurrence of {} is "
                              "replaced with observable signature name. If not specified, '_{}.txt' is appended at the "
                              "end",
         cxxopts::value<std::string>(bulkObsOutputFilename))
        ("B,bulk-observables", "replays the simulation and calculates specified bulk observables (format as in the "
                               "input file). Observables can be passed using multiple short options (-o obs1 -o obs2) "
                               "or comma-separated in a long option (--observables=obs1,obs2)",
         cxxopts::value<std::vector<std::string>>(bulkObservables))
        ("a,averaging-start", "specifies when the averaging starts. It is used for bulk observables",
         cxxopts::value<std::size_t>(averagingStart))
        ("T,max-threads", "specifies maximal number of OpenMP threads that may be used to calculate observables. If 0 "
                          "is passed, all available threads are used",
         cxxopts::value<std::size_t>(maxThreads)->default_value("1"))
        ("d,generate-dat", "reads the last snapshot and recreates *.dat file from it",
         cxxopts::value<std::string>(datFilename))
        ("w,generate-wolfram", "reads the last snapshot and recreates Wolfram Mathematica file from it",
         cxxopts::value<std::string>(wolframFilename))
        ("I,log-info", "print basic information about the recorded trajectory on a standard output")
        ("l,log-file", "if specified, messages will be logged both on the standard output and to this file. "
                       "Verbosity defaults then to: warn for standard output and to: info for log file, unless "
                       "changed by --verbosity and/or --log-file-verbosity options",
         cxxopts::value<std::string>(auxOutput))
        ("log-file-verbosity", "how verbose the output to the log file should be. Allowed values, with increasing "
                               "verbosity: error, warn, info, verbose, debug. Defaults to: info",
         cxxopts::value<std::string>(auxVerbosity))
        ("s,store-trajectory", "store the trajectory; it is most useful for broken trajectories fixed using --auto-fix",
         cxxopts::value<std::string>(storeFilename));

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    std::optional<std::string> verbosityOptional;
    if (parsedOptions.count("verbosity"))
        verbosityOptional = verbosity;
    std::optional<std::string> auxOutputOptional;
    if (parsedOptions.count("log-file"))
        auxOutputOptional = auxOutput;
    std::optional<std::string> auxVerbosityOptional;
    if (parsedOptions.count("log-file-verbosity"))
        auxVerbosityOptional = auxVerbosity;
    this->setVerbosityLevel(verbosityOptional, auxOutputOptional, auxVerbosityOptional);

    // Validate parsed options
    std::string cmd(argv[0]);
    if (argc != 1)
        die("Unexpected positional arguments. See " + cmd + " --help", this->logger);
    if (!parsedOptions.count("input"))
        die("Input file must be specified with option -i [input file name]", this->logger);
    if (!parsedOptions.count("trajectory"))
        die("Trajectory file must be specified with option -t [input file name]", this->logger);
    if (!parsedOptions.count("observables") && !parsedOptions.count("bulk-observables")
        && !parsedOptions.count("log-info") && !parsedOptions.count("generate-dat")
        && !parsedOptions.count("generate-wolfram"))
    {
        die("At least one of: --observables, --bulk-observables, --log-info, --generate-dat, --generate-wolfram "
            "options must be specified", this->logger);
    }

    Parameters params = this->loadParameters(inputFilename);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    auto shapeTraits = ShapeFactory::shapeTraitsFor(params.shapeName, params.shapeAttributes, params.interaction);
    auto packing = ArrangementFactory::arrangePacking(params.numOfParticles, params.initialDimensions,
                                                      params.initialArrangement, std::move(bc),
                                                      shapeTraits->getInteraction(), shapeTraits->getGeometry(), 1, 1);
    this->createWalls(*packing, params.walls);

    bool autoFix = parsedOptions.count("auto-fix");
    auto player = this->loadSimulationPlayer(trajectoryFilename, packing->size(), autoFix);
    if (player == nullptr)
        return EXIT_FAILURE;

    // Show info (if desired)
    if (parsedOptions.count("log-info")) {
        this->logger.info() << "-- " << trajectoryFilename << std::endl;
        player->dumpHeader(this->logger);
        this->logger.info() << std::endl;
    }

    // Stored trajectory (if desired)
    if (parsedOptions.count("store-trajectory")) {
        if (storeFilename == trajectoryFilename)
            die("Input and output trajectory file names are identical!", this->logger);

        bool isContinuation = false;
        auto recorder = this->loadSimulationRecorder(storeFilename, player->getNumMolecules(), player->getCycleStep(),
                                                     isContinuation);

        this->logger.info() << "Storing fixed trajectory started..." << std::endl;

        using namespace std::chrono;
        auto start = high_resolution_clock::now();
        while (player->hasNext()) {
            player->nextSnapshot(*packing, shapeTraits->getInteraction());
            recorder->recordSnapshot(*packing, player->getCurrentSnapshotCycles());
            this->logger.info() << "Replayed cycle " << player->getCurrentSnapshotCycles() << "; " << std::endl;
        }
        auto end = high_resolution_clock::now();
        double time = duration<double>(end - start).count();

        this->logger.info() << "Storing finished after " << time << " s." << std::endl;
        this->logger.info() << std::endl;

        player->reset();
    }

    // Replay the simulation and calculate observables (if desired)
    if (parsedOptions.count("observables")) {
        if (!parsedOptions.count("obs-output"))
            die("Output file for observables must be specified with option -o [output file name]", this->logger);

        this->logger.info() << "Starting simulation replay for observables..." << std::endl;
        auto collector = ObservablesCollectorFactory::create(observables, {}, maxThreads);

        using namespace std::chrono;
        auto start = high_resolution_clock::now();
        while (player->hasNext()) {
            player->nextSnapshot(*packing, shapeTraits->getInteraction());
            collector->addSnapshot(*packing, player->getCurrentSnapshotCycles(), *shapeTraits);
            this->logger.info() << "Replayed cycle " << player->getCurrentSnapshotCycles() << "; ";
            this->logger << collector->generateInlineObservablesString(*packing, *shapeTraits);
            this->logger << std::endl;
        }
        auto end = high_resolution_clock::now();
        double time = duration<double>(end - start).count();

        this->logger.info() << "Replay finished after " << time << " s." << std::endl;
        this->storeSnapshots(*collector, false, obsOutputFilename);
        this->logger.info() << std::endl;
    }

    // Replay the simulation and calculate bulk observables (if desired)
    if (parsedOptions.count("bulk-observables")) {
        if (!parsedOptions.count("bulk-obs-output"))
            die("Output file for bulk observables must be specified with option -b [output file name]", this->logger);
        if (!parsedOptions.count("averaging-start"))
            die("The start of averaging must be specified with option -a [first cycle number]", this->logger);
        if (averagingStart >= player->getTotalCycles()) {
            die("Starting cycle (" + std::to_string(averagingStart) + ") is larger than a total number of recorded "
                + "cycles (" + std::to_string(player->getTotalCycles()) + ")");
        }
        if (averagingStart < player->getCycleStep()) {
            die("Starting cycle (" + std::to_string(averagingStart) + ") is less than cycle number of first recorded "
                + "shapshot (" + std::to_string(player->getCycleStep()) + ")");
        }

        this->logger.info() << "Starting simulation replay for bulk observables..." << std::endl;
        auto collector = ObservablesCollectorFactory::create({}, bulkObservables, maxThreads);

        using namespace std::chrono;
        auto start = high_resolution_clock::now();

        // Round the starting cycle up
        std::size_t startingCycle{};
        std::size_t cycleStep = player->getCycleStep();
        if (averagingStart % cycleStep == 0)
            startingCycle = averagingStart;
        else
            startingCycle = (averagingStart / cycleStep + 1) * cycleStep;

        player->jumpToSnapshot(*packing, shapeTraits->getInteraction(), startingCycle);
        collector->addAveragingValues(*packing, *shapeTraits);
        this->logger.info() << "Replayed cycle " << player->getCurrentSnapshotCycles() << "; " << std::endl;
        while (player->hasNext()) {
            player->nextSnapshot(*packing, shapeTraits->getInteraction());
            collector->addAveragingValues(*packing, *shapeTraits);
            this->logger.info() << "Replayed cycle " << player->getCurrentSnapshotCycles() << "; " << std::endl;
        }
        auto end = high_resolution_clock::now();
        double time = duration<double>(end - start).count();

        this->logger.info() << "Replay finished after " << time << " s." << std::endl;
        this->storeBulkObservables(*collector, bulkObsOutputFilename);
        this->logger.info() << std::endl;
    }

    // Recreate dat file from the last snapshot (if desired)
    if (parsedOptions.count("generate-dat") || parsedOptions.count("generate-wolfram")) {
        this->logger.info() << "Reading last snapshot... " << std::flush;
        player->lastSnapshot(*packing, shapeTraits->getInteraction());
        this->logger.info() << "cycles: " << player->getCurrentSnapshotCycles() << std::endl;

        if (parsedOptions.count("generate-dat"))
            this->generateDatFile(*packing, params, *shapeTraits, datFilename, player->getCurrentSnapshotCycles());
        if (parsedOptions.count("generate-wolfram"))
            this->storeWolframVisualization(*packing, shapeTraits->getPrinter(), wolframFilename);
        this->logger.info() << std::endl;
    }

    return EXIT_SUCCESS;
}

std::unique_ptr<SimulationPlayer> Frontend::loadSimulationPlayer(std::string &trajectoryFilename,
                                                                 std::size_t numMolecules, bool autoFix_)
{
    auto trajectoryStream = std::make_unique<std::ifstream>(trajectoryFilename,
                                                            std::ios_base::in | std::ios_base::binary);
    ValidateOpenedDesc(*trajectoryStream, trajectoryFilename, "to read trajectory");
    if (autoFix_) {
        SimulationPlayer::AutoFix autoFix(numMolecules);
        try {
            auto simulationPlayer = std::make_unique<SimulationPlayer>(std::move(trajectoryStream), autoFix);
            autoFix.dumpInfo(this->logger);
            this->logger.info() << std::endl;
            return simulationPlayer;
        } catch (const ValidationException &exception) {
            autoFix.dumpInfo(this->logger);
            return nullptr;
        }
    } else {
        std::unique_ptr<SimulationPlayer> player;
        try {
            player = std::make_unique<SimulationPlayer>(std::move(trajectoryStream));
        } catch (const ValidationException &exception) {
            this->logger.error() << "Reading the trajectory failed: " << exception.what() << std::endl;
            this->logger << "You may try to fix it by adding the --auto-fix option." << std::endl;
            return nullptr;
        }
        Validate(player->getNumMolecules() == numMolecules);
        return player;
    }
}

void Frontend::createWalls(Packing &packing, const std::string &walls) {
    for (char c : walls) {
        if (std::isspace(c))
            continue;
        switch (std::tolower(c)) {
            case 'x':   packing.toggleWall(0, true);    break;
            case 'y':   packing.toggleWall(1, true);    break;
            case 'z':   packing.toggleWall(2, true);    break;
            default:    throw ValidationException("unknown wall axis: " + std::string{c});
        }
    }
}

void Frontend::storeBulkObservables(const ObservablesCollector &observablesCollector,
                                    std::string bulkObservableFilenamePattern) const
{
    std::regex pattern(R"(\{\})");
    if (!std::regex_search(bulkObservableFilenamePattern, pattern))
        bulkObservableFilenamePattern = bulkObservableFilenamePattern + "_{}.txt";

    observablesCollector.visitBulkObservables([&](const BulkObservable &bulkObservable) {
        std::string observableName = bulkObservable.getSignatureName();
        std::string filename = std::regex_replace(bulkObservableFilenamePattern, pattern, observableName);
        std::ofstream out(filename);
        ValidateOpenedDesc(out, filename, "to store bulk observable");
        bulkObservable.print(out);
        this->logger.info() << "Bulk observable " << observableName << " stored to " << filename << std::endl;
    });
}

int Frontend::shapePreview(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Information and preview for the shape.");

    std::string inputFilename;
    std::string shapeName;
    std::string shapeAttr;
    std::string interactionName;
    std::string wolframFilename;

    options.add_options()
        ("h,help", "prints help for this mode")
        ("i,input", "an INI file with parameters of the shape; it can be used instead of manually "
                    "specifying shape parameters using -S, -A and -I",
         cxxopts::value<std::string>(inputFilename))
        ("S,shape-name", "manually specified shape name (instead of reading from INI file using -i); it has "
                         "to be combined with -A and -I options",
         cxxopts::value<std::string>(shapeName))
        ("A,shape-attr", "manually specified shape attributes (instead of reading from INI file using -i); "
                         "it has to be combined with -S and -I options",
         cxxopts::value<std::string>(shapeAttr))
        ("I,interaction", "manually specified interactionName (instead of reading from INI file using -i); it "
                          "has to be combined with -S and -A options",
         cxxopts::value<std::string>(interactionName))
        ("l,log-info", "prints information about the shape")
        ("w,wolfram-preview", "stores Wolfram preview of the shape in a file given as an argument",
         cxxopts::value<std::string>(wolframFilename));

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    // Create shape traits
    if (parsedOptions.count("input")) {
        Parameters parameters = this->loadParameters(inputFilename);
        shapeName = parameters.shapeName;
        shapeAttr = parameters.shapeAttributes;
        interactionName = parameters.interaction;
    } else if (!parsedOptions.count("shape-name") || !parsedOptions.count("shape-attr")
               || !parsedOptions.count("interaction"))
    {
        die("You must specify INI file with shape parameters using -i or do it manually using -S, -A and -I",
            this->logger);
    }

    auto traits = ShapeFactory::shapeTraitsFor(shapeName, shapeAttr, interactionName);

    if (!parsedOptions.count("log-info") && !parsedOptions.count("wolfram-preview"))
        die("At least one of options: -l (--log-info), -W (--wolfram-preview) must be specified", this->logger);

    // Log info
    if (parsedOptions.count("log-info")) {
        this->logger.info() << "Shape name       : " << shapeName << std::endl;
        this->logger << "Shape attributes : " << shapeAttr << std::endl;
        this->logger << "Interaction      : " << interactionName << std::endl;
        this->logger << std::endl;
        this->printInteractionInfo(traits->getInteraction());
        this->logger << std::endl;
        this->printGeometryInfo(traits->getGeometry());
    }

    // Wolfram preview
    if (parsedOptions.count("wolfram-preview")) {
        std::ofstream wolframFile(wolframFilename);
        ValidateOpenedDesc(wolframFile, wolframFilename, " to store Wolfram preview of the shape");
        const auto &printer = traits->getPrinter();
        wolframFile << "Graphics3D[" << printer.toWolfram({}) << "]";
    }

    return EXIT_SUCCESS;
}

void Frontend::printGeometryInfo(const ShapeGeometry &geometry) {
    logger << "## Geometry info" << std::endl;
    logger << "Volume           : " << geometry.getVolume() << std::endl;
    logger << "Geometric origin : " << geometry.getGeometricOrigin({}) << std::endl;

    // Axes
    try {
        auto axis = geometry.getPrimaryAxis({});
        logger << "Primary axis     : " << axis << std::endl;
    } catch (std::runtime_error &) {
        logger << "Primary axis     : UNSPECIFIED" << std::endl;
    }
    try {
        auto axis = geometry.getSecondaryAxis({});
        logger << "Secondary axis   : " << axis << std::endl;
    } catch (std::runtime_error &) {
        logger << "Secondary axis   : UNSPECIFIED" << std::endl;
    }

    // Named points
    logger << "Named points     :" << std::endl;
    auto points = geometry.getNamedPoints();
    std::size_t maxLength = std::max_element(points.begin(), points.end(), [](const auto &p1, const auto &p2) {
        return p1.first.length() < p2.first.length();
    })->first.length();

    for (const auto &[name, point] : points) {
        logger << "    " << std::left << std::setw(maxLength) << name << " = " << point << std::endl;
    }
}

void Frontend::printInteractionInfo(const Interaction &interaction) {
    auto displayBool = [](bool b) { return b ? "true" : "false"; };
    logger << "## Interaction info" << std::endl;
    logger << "Has hard part            : " << displayBool(interaction.hasHardPart()) << std::endl;
    logger << "Has soft part            : " << displayBool(interaction.hasSoftPart()) << std::endl;
    logger << "Has wall part            : " << displayBool(interaction.hasWallPart()) << std::endl;
    logger << "Interaction center range : " << interaction.getRangeRadius() << std::endl;
    logger << "Total range              : " << interaction.getTotalRangeRadius() << std::endl;
    logger << "Interaction centers      :" << std::endl;
    auto interactionCentres = interaction.getInteractionCentres();
    if (interactionCentres.empty())
        interactionCentres.emplace_back();
    for (std::size_t i{}; i < interactionCentres.size(); i++)
        logger << "    [" << i << "] = " << interactionCentres[i] << std::endl;
}
