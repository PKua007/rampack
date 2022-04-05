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
#include "core/DistanceOptimizer.h"
#include "ArrangementFactory.h"
#include "TriclinicBoxScalerFactory.h"
#include "core/shapes/CompoundShapeTraits.h"
#include "MoveSamplerFactory.h"
#include "core/SimulationRecorder.h"
#include "core/SimulationPlayer.h"


Parameters Frontend::loadParameters(const std::string &inputFilename) {
    std::ifstream paramsFile(inputFilename);
    ValidateOpenedDesc(paramsFile, inputFilename, "to load input parameters");
    return Parameters(paramsFile);
}

void Frontend::setVerbosityLevel(const std::string &verbosityLevelName) const {
    if (verbosityLevelName == "error")
        this->logger.setVerbosityLevel(Logger::ERROR);
    else if (verbosityLevelName == "warn")
        this->logger.setVerbosityLevel(Logger::WARN);
    else if (verbosityLevelName == "info")
        this->logger.setVerbosityLevel(Logger::INFO);
    else if (verbosityLevelName == "verbose")
        this->logger.setVerbosityLevel(Logger::VERBOSE);
    else if (verbosityLevelName == "debug")
        this->logger.setVerbosityLevel(Logger::DEBUG);
    else
        die("Unknown verbosity level: " + verbosityLevelName, this->logger);
}

int Frontend::casino(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Monte Carlo sampling for both hard and soft potentials.");

    std::string inputFilename;
    std::string verbosity;
    std::string startFrom;
    std::size_t continuationCycles;

    options.add_options()
            ("h,help", "prints help for this mode")
            ("i,input", "an INI file with parameters. See sample_inputs folder for full parameters documentation",
             cxxopts::value<std::string>(inputFilename))
            ("V,verbosity", "how verbose the output should be. Allowed values, with increasing verbosity: "
                            "error, warn, info, verbose, debug",
             cxxopts::value<std::string>(verbosity)->default_value("info"))
            ("s,start-from", "when specified, the simulation will be started from the run with the name given. If not "
                             "used in conjunction with --continue option, the packing will be restored from the "
                             "internal representation file of the preceding run. If --continue is used, the current "
                             "run, but finished or aborted in the past, will be loaded instead",
             cxxopts::value<std::string>(startFrom))
            ("c,continue", "when specified, the thermalization of previously finished or aborted run will be continued "
                           "for as many more cycles as specified. It can be used together with --start-from to specify "
                           "which run should be continued. If the thermalization phase is already over, the error will "
                           "be reported. If 0 is specified (or left blank, since 0 is the default value), "
                           "total number of thermalization cycles from the input file will not be changed",
             cxxopts::value<std::size_t>(continuationCycles)->implicit_value("0"));

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    this->setVerbosityLevel(verbosity);

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
        moveSamplers.push_back(MoveSamplerFactory::create(moveSamplerString));

    // Find starting run index if specified
    std::size_t startRunIndex{};
    if (parsedOptions.count("start-from")) {
        auto runsParameters = params.runsParameters;
        auto nameMatchesStartFrom = [startFrom](const auto &params) {
            auto runNameGetter = [](auto &&run) { return run.runName; };
            return std::visit(runNameGetter, params) == startFrom;
        };
        auto it = std::find_if(runsParameters.begin(), runsParameters.end(), nameMatchesStartFrom);

        ValidateMsg(it != runsParameters.end(), "Invalid run name to start from");
        startRunIndex = it - runsParameters.begin();
    }

    // Load starting state from a previous or current run packing depending on --start-from and --continue
    // options combination
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    std::unique_ptr<Packing> packing;

    std::size_t cycleOffset{};  // Non-zero, if continuing previous run
    bool isContinuation{};
    if ((parsedOptions.count("start-from") && startRunIndex != 0) || parsedOptions.count("continue")) {
        auto &runsParameters = params.runsParameters;
        std::size_t startingPackingRunIndex{};
        if (parsedOptions.count("continue"))
            startingPackingRunIndex = startRunIndex;
        else
            startingPackingRunIndex = startRunIndex - 1;
        // A run, whose resulting packing will be the starting point
        auto &startingPackingRun = runsParameters[startingPackingRunIndex];

        auto packingFilenameGetter = [](auto &&run) { return run.packingFilename; };
        std::string previousPackingFilename = std::visit(packingFilenameGetter, startingPackingRun);
        std::ifstream packingFile(previousPackingFilename);
        ValidateOpenedDesc(packingFile, previousPackingFilename, "to load previous packing");
        // Same number of scaling and domain decemposition threads
        packing = std::make_unique<Packing>(std::move(bc), scalingThreads, scalingThreads);
        auto auxInfo = packing->restore(packingFile, shapeTraits->getInteraction());

        this->overwriteMoveStepSizes(moveSamplers, auxInfo);
        std::string scalingKey = Frontend::formatMoveKey("scaling", "scaling");
        params.volumeStepSize = std::stod(auxInfo.at(scalingKey));
        Validate(params.volumeStepSize > 0);

        if (parsedOptions.count("continue")) {
            cycleOffset = std::stoul(auxInfo.at("cycles"));
            isContinuation = true;

            // Value of continuation cycles is only used in integration mode. For overlaps rejection it is redundant
            if (std::holds_alternative<Parameters::IntegrationParameters>(startingPackingRun)) {
                // Because we continue this already finished run
                auto &startingRun = std::get<Parameters::IntegrationParameters>(startingPackingRun);

                if (continuationCycles == 0)
                    continuationCycles = startingRun.thermalisationCycles;

                if (continuationCycles <= cycleOffset) {
                    startingRun.thermalisationCycles = 0;
                    this->logger.info() << "Thermalisation of the finished run '" << startingRun.runName;
                    this->logger << "' will be skipped, since " << continuationCycles << " or more cycles were ";
                    this->logger << "already performed." << std::endl;
                } else {
                    startingRun.thermalisationCycles = continuationCycles - cycleOffset;
                    this->logger.info() << "Thermalisation from the finished run '" << startingRun.runName;
                    this->logger << "' will be continued up to " << continuationCycles << " cycles (";
                    this->logger << startingRun.thermalisationCycles << " to go)" << std::endl;
                }
            }
        }

        this->logger.info() << "Loaded packing from the run '" << previousPackingFilename << "' as a starting point.";
        this->logger << std::endl;
    }

    // If packing was not loaded from file, arrange it as given in config file
    if (packing == nullptr) {
        std::array<double, 3> dimensions = this->parseDimensions(params.initialDimensions);
        // Same number of scaling and domain threads
        packing = ArrangementFactory::arrangePacking(params.numOfParticles, dimensions, params.initialArrangement,
                                                     std::move(bc), shapeTraits->getInteraction(), scalingThreads,
                                                     scalingThreads);
    }

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
    if (!runParams.recordingFilename.empty())
        recorder = loadSimulationRecorder(runParams.recordingFilename, isContinuation);

    auto collector = ObservablesCollectorFactory::create(explode(runParams.observables, ','));
    simulation.integrate(runParams.temperature, runParams.pressure, runParams.thermalisationCycles,
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
        this->storeWolframVisualization(simulation, shapeTraits, runParams.wolframFilename);
    if (!runParams.outputFilename.empty()) {
        this->storeAverageValues(runParams.outputFilename, observablesCollector, runParams.temperature,
                                 runParams.pressure);
    }
    if (!runParams.observableSnapshotFilename.empty())
        this->storeSnapshots(observablesCollector, isContinuation, runParams.observableSnapshotFilename);
}

std::unique_ptr<SimulationRecorder> Frontend::loadSimulationRecorder(const std::string &filename,
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
    return std::make_unique<SimulationRecorder>(std::move(inout), isContinuation);
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
    if (!runParams.recordingFilename.empty())
        recorder = loadSimulationRecorder(runParams.recordingFilename, isContinuation);

    if (!runParams.helperInteraction.empty()) {
        auto helperShape = ShapeFactory::shapeTraitsFor(shapeName, shapeAttr, runParams.helperInteraction);
        shapeTraits = std::make_shared<CompoundShapeTraits>(shapeTraits, helperShape);
    }

    auto collector = ObservablesCollectorFactory::create(explode(runParams.observables, ','));
    simulation.relaxOverlaps(runParams.temperature, runParams.pressure, runParams.snapshotEvery, *shapeTraits,
                             std::move(collector), std::move(recorder), this->logger, cycleOffset);
    const ObservablesCollector &observablesCollector = simulation.getObservablesCollector();

    this->logger.info();
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->printPerformanceInfo(simulation);

    if (!runParams.packingFilename.empty())
        this->storePacking(simulation, runParams.packingFilename);
    if (!runParams.wolframFilename.empty())
        this->storeWolframVisualization(simulation, *shapeTraits, runParams.wolframFilename);
    if (!runParams.observableSnapshotFilename.empty())
        this->storeSnapshots(observablesCollector, isContinuation, runParams.observableSnapshotFilename);
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

void Frontend::storeWolframVisualization(const Simulation &simulation, const ShapeTraits &shapeTraits,
                                         const std::string &wolframFilename) const
{
    std::ofstream out(wolframFilename);
    ValidateOpenedDesc(out, wolframFilename, "to store Wolfram packing");
    simulation.getPacking().toWolfram(out, shapeTraits.getPrinter());
    this->logger.info() << "Wolfram packing stored to " + wolframFilename << std::endl;
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
    std::array<double, 3> dimensions = this->parseDimensions(params.initialDimensions);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    auto shapeTraits = ShapeFactory::shapeTraitsFor(params.shapeName, params.shapeAttributes, params.interaction);
    auto packing = ArrangementFactory::arrangePacking(params.numOfParticles, dimensions, params.initialArrangement,
                                                      std::move(bc), shapeTraits->getInteraction(), 1, 1);

    // Parse move type
    auto moveSamplerStrings = explode(params.moveTypes, ',');
    std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
    moveSamplers.reserve(moveSamplerStrings.size());
    for (const auto &moveSamplerString : moveSamplerStrings)
        moveSamplers.push_back(MoveSamplerFactory::create(moveSamplerString));

    // Store packing (if desired)
    if (parsedOptions.count("dat")) {
        std::map<std::string, std::string> auxInfo;
        this->appendMoveStepSizesToAuxInfo(moveSamplers, params.volumeStepSize, auxInfo);
        auxInfo["cycles"] = "0";

        std::ofstream out(datFilename);
        ValidateOpenedDesc(out, datFilename, "to store packing data");
        packing->store(out, auxInfo);
        this->logger.info() << "Packing stored to " + datFilename << std::endl;
    }

    // Store Mathematica packing (if desired)
    if (parsedOptions.count("wolfram")) {
        std::ofstream out(wolframFilename);
        ValidateOpenedDesc(out, wolframFilename, "to store Wolfram packing");
        packing->toWolfram(out, shapeTraits->getPrinter());
        this->logger.info() << "Wolfram packing stored to " + wolframFilename << std::endl;
    }

    return EXIT_SUCCESS;
}

std::array<double, 3> Frontend::parseDimensions(const std::string &initialDimensions) const {
    std::istringstream dimensionsStream(initialDimensions);
    std::array<double, 3> dimensions{};
    if (initialDimensions.find("auto") != std::string::npos) {
        std::string autoStr;
        dimensionsStream >> autoStr;
        ValidateMsg(dimensionsStream && autoStr == "auto", "Invalid packing dimensions format. "
                                                           "Expected: {auto|[dim x] [dim y] [dim z]}");
        dimensions.fill(0);
    } else {
        dimensionsStream >> dimensions[0] >> dimensions[1] >> dimensions[2];
        ValidateMsg(dimensionsStream, "Invalid packing dimensions format. "
                                      "Expected: {auto|[dim x] [dim y] [dim z]}");
        Validate(std::all_of(dimensions.begin(), dimensions.end(), [](double d) { return d > 0; }));
    }
    return dimensions;
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
    std::string outputFilename;
    std::vector<std::string> observables;
    std::string verbosity;

    options.add_options()
        ("h,help", "prints help for this mode")
        ("i,input", "an INI file with parameters that was used to generate the trajectories. See sample_inputs "
                    "folder for full parameters documentation",
         cxxopts::value<std::string>(inputFilename))
        ("t,trajectory", "a file with recorder trajectory",
         cxxopts::value<std::string>(trajectoryFilename))
        ("V,verbosity", "how verbose the output should be. Allowed values, with increasing verbosity: "
                        "error, warn, info, verbose, debug",
         cxxopts::value<std::string>(verbosity)->default_value("info"))
        ("o,output", "output file with the results (depending of a selected mode, for example --observables)",
         cxxopts::value<std::string>(outputFilename))
        ("O,observables", "replays the simulation and calculates specified observables (format as in the input file). "
                          "Observables can be passed using multiple short options (-o obs1 -o obs2) or comma-separated "
                          "in a long option (--observables=obs1,obs2)",
         cxxopts::value<std::vector<std::string>>(observables))
        ("l,log-info", "print basic information about the recorded trajectory on a standard output");

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    this->setVerbosityLevel(verbosity);

    // Validate parsed options
    std::string cmd(argv[0]);
    if (argc != 1)
        die("Unexpected positional arguments. See " + cmd + " --help", this->logger);
    if (!parsedOptions.count("input"))
        die("Input file must be specified with option -i [input file name]", this->logger);
    if (!parsedOptions.count("trajectory"))
        die("Trajectory file must be specified with option -t [input file name]", this->logger);
    if (!parsedOptions.count("observables") && !parsedOptions.count("log-info"))
        die("At least one of: --observables, --log-info options must be specified", this->logger);

    Parameters params = this->loadParameters(inputFilename);
    std::array<double, 3> dimensions = this->parseDimensions(params.initialDimensions);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    auto shapeTraits = ShapeFactory::shapeTraitsFor(params.shapeName, params.shapeAttributes, params.interaction);
    auto packing = ArrangementFactory::arrangePacking(params.numOfParticles, dimensions, params.initialArrangement,
                                                      std::move(bc), shapeTraits->getInteraction(), 1, 1);

    auto trajectoryStream = std::make_unique<std::ifstream>(trajectoryFilename,
                                                                         std::ios_base::in | std::ios_base::binary);
    ValidateOpenedDesc(*trajectoryStream, trajectoryFilename, "to read trajectory");
    SimulationPlayer player(std::move(trajectoryStream));

    // Show info (if desired)
    if (parsedOptions.count("log-info")) {
        this->logger.info() << "-- " << trajectoryFilename << std::endl;
        player.dumpHeader(this->logger);
        this->logger.info() << std::endl;
    }

    // Replay the simulation and calculate observables (if desired)
    if (parsedOptions.count("observables")) {
        if (!parsedOptions.count("output"))
            die("Output file must be specified with option -o [output file name]", this->logger);

        this->logger.info() << "Starting simulation replay..." << std::endl;
        auto collector = ObservablesCollectorFactory::create(observables);

        using namespace std::chrono;
        auto start = high_resolution_clock::now();
        while (player.hasNext()) {
            player.nextSnapshot(*packing, shapeTraits->getInteraction());
            collector->addSnapshot(*packing, player.getCurrentSnapshotCycles(), *shapeTraits);
            this->logger.info() << "Replayed cycle " << player.getCurrentSnapshotCycles() << "; ";
            this->logger << collector->generateInlineObservablesString(*packing, *shapeTraits);
            this->logger << std::endl;
        }
        auto end = high_resolution_clock::now();
        double time = duration<double>(end - start).count();

        this->logger.info() << "Replay finished after " << time << " s." << std::endl;
        this->storeSnapshots(*collector, false, outputFilename);
    }

    return EXIT_SUCCESS;
}
