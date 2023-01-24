//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cstdlib>
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
#include "legacy/ObservablesCollectorFactory.h"
#include "core/Simulation.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/Packing.h"
#include "utils/OMPMacros.h"
#include "core/shapes/CompoundShapeTraits.h"
#include "core/io/RamtrjPlayer.h"
#include "PackingLoader.h"
#include "legacy/ParameterUpdaterFactory.h"
#include "core/io/XYZRecorder.h"
#include "core/io/TruncatedPlayer.h"
#include "utils/ParseUtils.h"
#include "matchers/ShapeMatcher.h"
#include "pyon/Parser.h"
#include "matchers/ObservablesMatcher.h"
#include "RampackParameters.h"
#include "legacy/IniParametersFactory.h"
#include "matchers/RampackMatcher.h"
#include "matchers/FileSnapshotWriterMatcher.h"
#include "matchers/FileShapePrinterMatcher.h"
#include "matchers/SimulationRecorderFactoryMatcher.h"


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
            ("i,input", "an INI/PYON file with parameters. See sample_inputs folder for full parameters documentation",
             cxxopts::value<std::string>(inputFilename))
            ("V,verbosity", "how verbose the output should be. Allowed values, with increasing verbosity: "
                            "error, warn, info, verbose, debug. Defaults to: info if --log-file not specified, "
                            "otherwise to: warn",
             cxxopts::value<std::string>(verbosity))
            ("s,start-from", "when specified, the simulation will be started from the run with the name given. If not "
                             "used in conjunction with --continue option, the packing will be restored from the "
                             "RAMSNAP file of the preceding run. If --continue is used, the current run, but finished "
                             "or aborted in the past, will be loaded instead. There are also some special values. "
                             "'.start' and '.end' correspond to, respectively, first and last run in the "
                             "configuration file. When a special value '.auto' is specified, auto-detection of the "
                             "starting run will be attempted based on RAMSNAP files (all runs in configuration have to "
                             "output them). If last attempted run was unfinished, --continue option without argument "
                             "is implicitly added",
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
    if (!parsedOptions.unmatched().empty())
        die("Unexpected positional arguments. See " + cmd + " --help", this->logger);
    if (!parsedOptions.count("input"))
        die("Input file must be specified with option -i [input file name]", this->logger);

    // Load parameters
    this->logger.info();
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "Random And Maximal PACKing PACKage v" << CURRENT_VERSION << std::endl;
    this->logger << "(C) 2023 Piotr Kubala and Collaborators" << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;

    RampackParameters rampackParams = this->dispatchParams(inputFilename);
    const auto &baseParams = rampackParams.baseParameters;
    const auto &shapeTraits = baseParams.shapeTraits;

    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "Interaction centre range : " << shapeTraits->getInteraction().getRangeRadius() << std::endl;
    this->logger << "Total interaction range  : " << shapeTraits->getInteraction().getTotalRangeRadius() << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;

    std::size_t numDomains = std::accumulate(baseParams.domainDivisions.begin(), baseParams.domainDivisions.end(), 1ul,
                                             std::multiplies<>{});

    // We use the same number of threads for scaling and particle moves, otherwise OpenMP leaks memory
    // Too many domain threads are ok, some will just be jobless. But we cannot use less scaling threads than
    // domain threads
    // See https://stackoverflow.com/questions/67267035/...
    // ...increasing-memory-consumption-for-2-alternating-openmp-parallel-regions-with-dif
    Validate(numDomains <= baseParams.scalingThreads);

    // Info about threads
    this->logger << OMP_MAXTHREADS << " OpenMP threads are available" << std::endl;
    this->logger << "Using " << baseParams.scalingThreads << " threads for scaling moves" << std::endl;
    if (numDomains == 1) {
        this->logger << "Using 1 thread without domain decomposition for particle moves" << std::endl;
    } else {
        this->logger << "Using " << baseParams.domainDivisions[0] << " x " << baseParams.domainDivisions[1] << " x ";
        this->logger << baseParams.domainDivisions[2] << " = " << numDomains << " domains for particle moves" << std::endl;
    }
    this->logger << "--------------------------------------------------------------------" << std::endl;

    // Load starting state from a previous or current run packing depending on --start-from and --continue
    // options combination

    std::optional<std::string> optionalStartFrom;
    std::optional<std::size_t> optionalContinuationCycles;
    if (parsedOptions.count("start-from"))
        optionalStartFrom = startFrom;
    if (parsedOptions.count("continue"))
        optionalContinuationCycles = continuationCycles;

    PackingLoader packingLoader(this->logger, optionalStartFrom, optionalContinuationCycles, rampackParams.runs);
    auto packing = this->recreatePacking(packingLoader, rampackParams.baseParameters, *shapeTraits,
                                         baseParams.scalingThreads);

    if (packingLoader.isAllFinished()) {
        this->logger.warn() << "No runs left to be performed. Exiting." << std::endl;
        return EXIT_SUCCESS;
    }

    std::size_t startRunIndex = packingLoader.getStartRunIndex();
    std::size_t cycleOffset = packingLoader.getCycleOffset();
    bool isContinuation = packingLoader.isContinuation();

    auto env = this->recreateEnvironment(rampackParams, packingLoader);

    // Perform simulations starting from initial run
    Simulation simulation(std::move(packing), baseParams.seed, baseParams.domainDivisions, baseParams.saveOnSignal);

    for (std::size_t i = startRunIndex; i < rampackParams.runs.size(); i++) {
        const auto &run = rampackParams.runs[i];
        // Environment for starting run is already prepared
        if (i != startRunIndex)
            Frontend::combineEnvironment(env, run);

        if (std::holds_alternative<IntegrationRun>(run)) {
            const auto &integrationRun = std::get<IntegrationRun>(run);
            this->verifyDynamicParameter(env.getTemperature(), "temperature", integrationRun, cycleOffset);
            if (env.isBoxScalingEnabled())
                this->verifyDynamicParameter(env.getPressure(), "pressure", integrationRun, cycleOffset);
            this->performIntegration(simulation, env, integrationRun, *shapeTraits, cycleOffset, isContinuation);
        } else if (std::holds_alternative<OverlapRelaxationRun>(run)) {
            const auto &overlapRelaxationRun = std::get<OverlapRelaxationRun>(run);
            this->performOverlapRelaxation(simulation, env, overlapRelaxationRun, shapeTraits, cycleOffset,
                                           isContinuation);
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

void Frontend::combineEnvironment(Simulation::Environment &env, const Run &run)
{
    auto environmentGetter = [](const auto &run) {
        return run.environment;
    };
    auto runEnv = std::visit(environmentGetter, run);
    env.combine(runEnv);
}

void Frontend::performIntegration(Simulation &simulation, Simulation::Environment &env, const IntegrationRun &run,
                                  const ShapeTraits &shapeTraits, std::size_t cycleOffset, bool isContinuation)
{
    this->logger.setAdditionalText(run.runName);
    this->logger.info() << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "Starting integration '" << run.runName << "'" << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;

    std::vector<std::unique_ptr<SimulationRecorder>> recorders;
    for (const auto &factory : run.simulationRecorders) {
        auto recorder = factory->create(simulation.getPacking().size(), run.snapshotEvery, isContinuation,
                                        this->logger);
        recorders.push_back(std::move(recorder));
    }

    auto collector = run.observablesCollector;
    if (run.observablesOut.has_value())
        this->attachSnapshotOut(*collector, *run.observablesOut, isContinuation);

    Simulation::IntegrationParameters integrationParams;
    integrationParams.thermalisationCycles = run.thermalizationCycles.value_or(0);
    integrationParams.averagingCycles = run.averagingCycles.value_or(0);
    integrationParams.averagingEvery = run.averagingEvery;
    integrationParams.snapshotEvery = run.snapshotEvery;
    integrationParams.inlineInfoEvery = run.inlineInfoEvery;
    integrationParams.rotationMatrixFixEvery = run.orientationFixEvery;
    integrationParams.cycleOffset = cycleOffset;

    simulation.integrate(env, integrationParams, shapeTraits, std::move(collector), std::move(recorders), this->logger);
    const ObservablesCollector &observablesCollector = simulation.getObservablesCollector();

    this->logger.info() << "--------------------------------------------------------------------" << std::endl;
    if (integrationParams.averagingCycles != 0 && !simulation.wasInterrupted()) {
        this->printAverageValues(observablesCollector);
    } else {
        this->logger.warn() << "Printing averages skipped due to incomplete averaging phase." << std::endl;
        this->logger.info() << "--------------------------------------------------------------------" << std::endl;
    }

    this->printPerformanceInfo(simulation);

    for (const auto &writer : run.lastSnapshotWriters)
        writer.storeSnapshot(simulation, shapeTraits, this->logger);

    if (run.averagesOut.has_value()) {
        if (integrationParams.averagingCycles != 0 && !simulation.wasInterrupted()) {
            this->storeAverageValues(*run.averagesOut, observablesCollector, simulation.getCurrentTemperature(),
                                     simulation.getCurrentPressure());
        } else {
            this->logger.warn() << "Storing averages skipped due to incomplete averaging phase." << std::endl;
        }
    }

    if (run.bulkObservablesOutPattern.has_value()) {
        if (integrationParams.averagingCycles != 0 && !simulation.wasInterrupted())
            this->storeBulkObservables(observablesCollector, *run.bulkObservablesOutPattern);
        else
            this->logger.warn() << "Storing bulk observables skipped due to incomplete averaging phase." << std::endl;
    }
}

void Frontend::performOverlapRelaxation(Simulation &simulation, Simulation::Environment &env,
                                        const OverlapRelaxationRun &run, std::shared_ptr<ShapeTraits> shapeTraits,
                                        std::size_t cycleOffset, bool isContinuation)
{
    this->logger.setAdditionalText(run.runName);
    this->logger.info() << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->logger << "Starting overlap relaxation '" << run.runName << "'" << std::endl;
    this->logger << "--------------------------------------------------------------------" << std::endl;

    std::vector<std::unique_ptr<SimulationRecorder>> recorders;
    for (const auto &factory : run.simulationRecorders) {
        auto recorder = factory->create(simulation.getPacking().size(), run.snapshotEvery, isContinuation,
                                        this->logger);
        recorders.push_back(std::move(recorder));
    }

    if (run.helperShapeTraits != nullptr)
        shapeTraits = std::make_shared<CompoundShapeTraits>(shapeTraits, run.helperShapeTraits);

    auto collector = run.observablesCollector;
    if (run.observablesOut.has_value())
        this->attachSnapshotOut(*collector, *run.observablesOut, isContinuation);

    Simulation::OverlapRelaxationParameters relaxParams;
    relaxParams.snapshotEvery = run.snapshotEvery;
    relaxParams.inlineInfoEvery = run.inlineInfoEvery;
    relaxParams.rotationMatrixFixEvery = run.orientationFixEvery;
    relaxParams.cycleOffset = cycleOffset;

    simulation.relaxOverlaps(env, relaxParams, *shapeTraits, std::move(collector), std::move(recorders), this->logger);

    this->logger.info();
    this->logger << "--------------------------------------------------------------------" << std::endl;
    this->printPerformanceInfo(simulation);

    for (const auto &writer : run.lastSnapshotWriters)
        writer.storeSnapshot(simulation, *shapeTraits, this->logger);
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
    double ngRebuildScalingPercent = (scalingSeconds == 0) ? 0 : (ngRebuildSeconds / scalingSeconds * 100);
    double domainDecompTotalPercent = domainDecompositionSeconds / totalSeconds * 100;
    double domainDecompMovePercent = domainDecompositionSeconds / moveSeconds * 100;
    double movePercent = moveSeconds / totalSeconds * 100;
    double scalingPercent = scalingSeconds / totalSeconds * 100;
    double observablesPercent = observablesSeconds / totalSeconds * 100;
    double otherPercent = otherSeconds / totalSeconds * 100;

    this->printMoveStatistics(simulation);
    this->logger.info() << "--------------------------------------------------------------------" << std::endl;
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
                   "models using Monte Carlo method.").width(80) << std::endl;
    rawOut << std::endl;
    rawOut << "Usage: " << cmd << " [mode] (mode dependent parameters). " << std::endl;
    rawOut << std::endl;
    rawOut << "Available modules:" << std::endl;
    rawOut << "casino" << std::endl;
    rawOut << Fold("Monte Carlo sampling for both hard and soft potentials.").width(80).margin(4) << std::endl;
    rawOut << "preview" << std::endl;
    rawOut << Fold("Based on the input file generate initial configuration and store it in a given format.")
              .width(80).margin(4) << std::endl;
    rawOut << "shape-preview" << std::endl;
    rawOut << Fold("Provides information and preview for a given shape.").width(80).margin(4) << std::endl;
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

int Frontend::preview(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Initial arrangement preview.");

    std::string inputFilename;
    std::vector<std::string> outputs;

    // TODO: refer to the documentation for -o format
    options.add_options()
        ("h,help", "prints help for this mode")
        ("i,input", "an INI/PYON file with parameters. See sample_inputs folder for full parameters documentation",
         cxxopts::value<std::string>(inputFilename))
        ("o,output", "output of the preview. Supported PYON classes: ramsnap, wolfram, xyz. More than one format can "
                     "be chosen by specifying this option multiple times",
         cxxopts::value<std::vector<std::string>>(outputs));

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    // Validate parsed options
    std::string cmd(argv[0]);
    if (!parsedOptions.unmatched().empty())
        die("Unexpected positional arguments. See " + cmd + " --help", this->logger);
    if (!parsedOptions.count("input"))
        die("Input file must be specified with option -i [input file name]", this->logger);
    if (outputs.empty())
        die("Option -o (--output) must be specified at least once", this->logger);

    RampackParameters params = this->dispatchParams(inputFilename);
    const auto &baseParams = params.baseParameters;
    auto shapeTraits = baseParams.shapeTraits;
    auto packingFactory = baseParams.packingFactory;
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    auto packing = packingFactory->createPacking(std::move(pbc), *shapeTraits, 1, 1);
    this->createWalls(*packing, baseParams.walls);

    for (const auto &output : outputs) {
        auto writer = Frontend::createFileSnapshotWriter(output);
        writer.generateSnapshot(*packing, *shapeTraits, 0, this->logger);
    }

    return EXIT_SUCCESS;
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

void Frontend::overwriteMoveStepSizes(Simulation::Environment &env,
                                      const std::map<std::string, std::string> &packingAuxInfo) const
{
    std::set<std::string> notUsedStepSizes;
    for (const auto &[key, value] : packingAuxInfo)
        if (Frontend::isStepSizeKey(key))
            notUsedStepSizes.insert(key);

    for (auto &moveSampler : env.getMoveSamplers()) {
        auto groupName = moveSampler->getName();
        for (const auto &[moveName, stepSize] : moveSampler->getStepSizes()) {
            std::string moveKey = Frontend::formatMoveKey(groupName, moveName);
            if (packingAuxInfo.find(moveKey) == packingAuxInfo.end()) {
                this->logger.warn() << "Step size " << moveKey << " not found in RAMSNAP metadata. Falling back to ";
                this->logger << "input file value " << stepSize << std::endl;
                continue;
            }

            notUsedStepSizes.erase(moveKey);
            moveSampler->setStepSize(moveName, std::stod(packingAuxInfo.at(moveKey)));
        }
    }

    if (env.isBoxScalingEnabled()) {
        std::string scalingKey = Frontend::formatMoveKey("scaling", "scaling");
        if (packingAuxInfo.find(scalingKey) == packingAuxInfo.end()) {
            this->logger.warn() << "Step size " << scalingKey << " not found in RAMSNAP metadata. Falling back to ";
            this->logger << "input file value " << env.getBoxScaler().getStepSize() << std::endl;
        } else {
            double volumeStepSize = std::stod(packingAuxInfo.at(scalingKey));
            Validate(volumeStepSize > 0);
            env.getBoxScaler().setStepSize(volumeStepSize);
            notUsedStepSizes.erase(scalingKey);
        }
    }

    if (notUsedStepSizes.empty())
        return;

    this->logger.warn() << "Packing RAMSNAP file contained metadata for unused move step sizes:" << std::endl;
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

int Frontend::trajectory(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Recorded trajectories analyzer.");

    std::string inputFilename;
    std::string runName;
    std::string obsOutputFilename;
    std::vector<std::string> observables;
    std::string bulkObsOutputFilename;
    std::vector<std::string> bulkObservables;
    std::size_t averagingStart{};
    std::size_t maxThreads{};
    std::vector<std::string> snapshotOutputs;
    std::string verbosity;
    std::string auxOutput;
    std::string auxVerbosity;
    std::vector<std::string> trajectoryOutputs;
    std::size_t truncatedCycles;

    // TODO: -s and -r documentation
    options.add_options()
        ("h,help", "prints help for this mode")
        ("i,input", "an INI file with parameters that was used to generate the trajectories. See sample_inputs "
                    "folder for full parameters documentation",
         cxxopts::value<std::string>(inputFilename))
        ("r,run-name", "name of the run, for which the trajectory was generated. Special values '.first' "
                       "and '.last' (for the first and the last run in the configuration file) are also accepted",
         cxxopts::value<std::string>(runName)->default_value(".last"))
        ("f,auto-fix", "tries to auto-fix the trajectory if it is broken; fixed trajectory can be stored back to "
                       "RAMTRJ using -t 'ramtrj(\"filename\")'")
        ("V,verbosity", "how verbose the output should be. Allowed values, with increasing verbosity: "
                        "error, warn, info, verbose, debug. Defaults to: info if --log-file not specified, "
                        "otherwise to: warn",
         cxxopts::value<std::string>(verbosity))
        ("o,output-obs", "calculate observables and output them to a given file. Observables can be specified using "
                         "-O (--observable)",
         cxxopts::value<std::string>(obsOutputFilename))
        ("O,observable", "replays the simulation and calculates specified observables (format as in the input file). "
                         "Observables can be passed using multiple options (-o obs1 -o obs2) or pipe-separated in a "
                         "single one (-o 'obs1|obs2')",
         cxxopts::value<std::vector<std::string>>(observables))
        ("b,output-bulk-obs", "calculate bulk observables and output them to the file with a name given by the "
                              "specified pattern. In the pattern, every occurrence of {} is replaced with observable "
                              "signature name. If not specified, '_{}.txt' is appended at the end. Bulk observables "
                              "are specified using -B (--bulk-observable)",
         cxxopts::value<std::string>(bulkObsOutputFilename))
        ("B,bulk-observable", "replays the simulation and calculates specified bulk observables (format as in the "
                              "input file). Observables can be passed using multiple options (-o obs1 -o obs2) "
                              "or pipe-separated in a single one (-o 'obs1|obs2')",
         cxxopts::value<std::vector<std::string>>(bulkObservables))
        ("a,averaging-start", "specifies when the averaging starts. It is used for bulk observables",
         cxxopts::value<std::size_t>(averagingStart))
        ("T,max-threads", "specifies maximal number of OpenMP threads that may be used to calculate observables. If 0 "
                          "is passed, all available threads are used",
         cxxopts::value<std::size_t>(maxThreads)->default_value("1"))
        ("s,output-snapshot", "reads the last snapshot and recreates outputs it in a given format: ramsnap, wolfram, "
                              "xyz. More than one output can be stored when multiple -s options are specified or in a "
                              "single one using pipe '|'",
         cxxopts::value<std::vector<std::string>>(snapshotOutputs))
        ("I,log-info", "print basic information about the recorded trajectory on a standard output")
        ("l,log-file", "if specified, messages will be logged both on the standard output and to this file. "
                       "Verbosity defaults then to: warn for standard output and to: info for log file, unless "
                       "changed by --verbosity and/or --log-file-verbosity options",
         cxxopts::value<std::string>(auxOutput))
        ("log-file-verbosity", "how verbose the output to the log file should be. Allowed values, with increasing "
                               "verbosity: error, warn, info, verbose, debug. Defaults to: info",
         cxxopts::value<std::string>(auxVerbosity))
        ("t,output-trajectory", "store the trajectory in a given format: ramtrj, xyz. More than one output can be "
                                "stored when multiple -t options are specified or in a single one using pipe '|'",
         cxxopts::value<std::vector<std::string>>(trajectoryOutputs))
        ("x,truncate", "truncates loaded trajectory to a given number of total cycles (trajectory file remains "
                       "unchanged); truncated trajectory can be stored to other RAMTRJ file using "
                       "-t 'ramtrj(\"filename\")'",
         cxxopts::value<std::size_t>(truncatedCycles));

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
    if (!parsedOptions.unmatched().empty())
        die("Unexpected positional arguments. See " + cmd + " --help", this->logger);
    if (!parsedOptions.count("input"))
        die("Input file must be specified with option -i [input file name]", this->logger);
    if (!parsedOptions.count("log-info") && !parsedOptions.count("output-obs")
        && !parsedOptions.count("output-bulk-obs") && !parsedOptions.count("output-snapshot")
        && !parsedOptions.count("output-trajectory"))
    {
        die("At least one of:\n"
            " -I (--log-info)\n"
            " -o (--output-obs)\n"
            " -b (--output-bulk-obs)\n"
            " -s (--output-snapshot)\n"
            " -t (--output-trajectory)\n"
            "options must be specified", this->logger);
    }

    if (runName == ".auto")
        die("'.auto' run is not supported in the trajectory mode", this->logger);

    // Prepare initial packing
    RampackParameters rampackParams = this->dispatchParams(inputFilename);
    const auto &baseParams = rampackParams.baseParameters;
    auto shapeTraits = baseParams.shapeTraits;
    auto packingFactory = baseParams.packingFactory;
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    auto packing = packingFactory->createPacking(std::move(pbc), *shapeTraits, maxThreads, maxThreads);
    this->createWalls(*packing, baseParams.walls);

    // Find and validate run whose trajectory we want to process
    std::size_t startRunIndex = PackingLoader::findStartRunIndex(runName, rampackParams.runs);
    const auto &startRun = rampackParams.runs[startRunIndex];

    auto ramtrjOut = std::visit([](auto &&run) { return run.ramtrjOut; }, startRun);
    std::string foundRunName = std::visit([](const auto &run) { return run.runName; }, startRun);

    if (!ramtrjOut.has_value())
        die("RAMTRJ trajectory was not recorded for the run '" + foundRunName + "'", this->logger);
    std::string trajectoryFilename = *ramtrjOut;

    if (foundRunName == runName) {
        this->logger.info() << "Trajectory of the run '" << foundRunName << "' will be processed" << std::endl;
    } else {
        this->logger.info() << "Trajectory of the run '" << runName << "' ('" << foundRunName << "') will be processed";
        this->logger << std::endl;
    }

    // Recreate environment
    auto environment = this->recreateRawEnvironment(rampackParams, startRunIndex);

    // Autofix trajectory if desired
    bool autoFix = parsedOptions.count("auto-fix");
    std::unique_ptr<SimulationPlayer> player = this->loadRamtrjPlayer(trajectoryFilename, packing->size(), autoFix);
    if (player == nullptr)
        return EXIT_FAILURE;

    // Show info (if desired)
    if (parsedOptions.count("log-info")) {
        this->logger.info() << "-- " << trajectoryFilename << std::endl;
        dynamic_cast<const RamtrjPlayer&>(*player).dumpHeader(this->logger);
        this->logger.info() << std::endl;
    }

    // Truncate trajectory (if desired)
    if (parsedOptions.count("truncate")) {
        ValidateMsg(truncatedCycles <= player->getTotalCycles(),
                    "Number of truncated cycles (" + std::to_string(truncatedCycles) + ") must not be greater than "
                    + "number of recorded cycles (" + std::to_string(player->getTotalCycles()));
        ValidateMsg(truncatedCycles % player->getCycleStep() == 0,
                    "Number of truncated cycles (" + std::to_string(truncatedCycles) + ") has to be divisible by the "
                    + "cycle step (" + std::to_string(player->getCycleStep()) + ")");

        auto truncatedPlayer = std::make_unique<TruncatedPlayer>(std::move(player), truncatedCycles);
        player = std::move(truncatedPlayer);
    }

    // Stored trajectory in RAMTRJ/Wolfram format (if desired)
    for (const auto &trajectoryOutput : trajectoryOutputs) {
        auto factory = Frontend::createSimulationRecorderFactory(trajectoryOutput);
        if (factory->getFilename() == trajectoryFilename)
            die("Input trajectory file name '" + trajectoryFilename + "' cannot be used as an output!", this->logger);

        bool isContinuation = false;
        auto recorder = factory->create(player->getNumMolecules(),
                                        player->getCycleStep(), isContinuation, this->logger);

        this->logger.info() << "Storing trajectory started..." << std::endl;

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
    if (parsedOptions.count("output-obs")) {
        if (observables.empty()) {
            die("When using -o (--output-obs), at least one observable should be specified: -O (--observable)",
                this->logger);
        }

        ObservablesCollector collector;
        for (const std::string &observable : observables) {
            auto observableData = Frontend::createObservable(observable, maxThreads);
            collector.addObservable(std::move(observableData.observable), observableData.scope);
        }

        this->logger.info() << "Starting simulation replay for observables..." << std::endl;

        using namespace std::chrono;
        auto start = high_resolution_clock::now();
        while (player->hasNext()) {
            player->nextSnapshot(*packing, shapeTraits->getInteraction());
            std::size_t cycles = player->getCurrentSnapshotCycles();
            std::size_t totalCycles = player->getTotalCycles();
            double temperature = environment.getTemperature().getValueForCycle(cycles, totalCycles);
            double pressure{};
            if (environment.isBoxScalingEnabled())
                pressure = environment.getPressure().getValueForCycle(cycles, totalCycles);
            collector.setThermodynamicParameters(temperature, pressure);
            collector.addSnapshot(*packing, player->getCurrentSnapshotCycles(), *shapeTraits);
            this->logger.info() << "Replayed cycle " << player->getCurrentSnapshotCycles() << "; ";
            this->logger << collector.generateInlineObservablesString(*packing, *shapeTraits);
            this->logger << std::endl;
        }
        auto end = high_resolution_clock::now();
        double time = duration<double>(end - start).count();

        this->logger.info() << "Replay finished after " << time << " s." << std::endl;
        this->storeSnapshots(collector, false, obsOutputFilename);
        this->logger.info() << std::endl;
    }

    // Replay the simulation and calculate bulk observables (if desired)
    if (parsedOptions.count("output-bulk-obs")) {
        if (bulkObservables.empty()) {
            die("When using -b (--output-bulk-obs), at least one observable should be specified: "
                "-B (--bulk-observable)", this->logger);
        }
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

        ObservablesCollector collector;
        for (const std::string &bulkObservable : bulkObservables) {
            auto theObservable = Frontend::createBulkObservable(bulkObservable, maxThreads);
            collector.addBulkObservable(theObservable);
        }

        this->logger.info() << "Starting simulation replay for bulk observables..." << std::endl;

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
        collector.addAveragingValues(*packing, *shapeTraits);
        this->logger.info() << "Replayed cycle " << player->getCurrentSnapshotCycles() << "; " << std::endl;
        while (player->hasNext()) {
            player->nextSnapshot(*packing, shapeTraits->getInteraction());
            collector.addAveragingValues(*packing, *shapeTraits);
            this->logger.info() << "Replayed cycle " << player->getCurrentSnapshotCycles() << "; " << std::endl;
        }
        auto end = high_resolution_clock::now();
        double time = duration<double>(end - start).count();

        this->logger.info() << "Replay finished after " << time << " s." << std::endl;
        this->storeBulkObservables(collector, bulkObsOutputFilename);
        this->logger.info() << std::endl;
    }

    // Recreate RAMSNAP/XYZ/Wolfram file from the last snapshot (if desired)
    if (!snapshotOutputs.empty()) {
        this->logger.info() << "Reading last snapshot... " << std::flush;
        player->lastSnapshot(*packing, shapeTraits->getInteraction());
        this->logger.info() << "cycles: " << player->getCurrentSnapshotCycles() << std::endl;

        for (const auto &snapshotOutput: snapshotOutputs) {
            auto writer = Frontend::createFileSnapshotWriter(snapshotOutput);
            if (writer.getFilename() == trajectoryFilename) {
                die("Input trajectory file name '" + trajectoryFilename + "' cannot be used as an output!",
                    this->logger);
            }

            writer.generateSnapshot(*packing, *shapeTraits, player->getCurrentSnapshotCycles(), this->logger);
        }
    }

    return EXIT_SUCCESS;
}

std::unique_ptr<RamtrjPlayer> Frontend::loadRamtrjPlayer(std::string &trajectoryFilename,
                                                         std::size_t numMolecules, bool autoFix_)
{
    auto trajectoryStream = std::make_unique<std::ifstream>(trajectoryFilename,
                                                            std::ios_base::in | std::ios_base::binary);
    ValidateOpenedDesc(*trajectoryStream, trajectoryFilename, "to read trajectory");
    if (autoFix_) {
        RamtrjPlayer::AutoFix autoFix(numMolecules);
        try {
            auto simulationPlayer = std::make_unique<RamtrjPlayer>(std::move(trajectoryStream), autoFix);
            autoFix.dumpInfo(this->logger);
            this->logger.info() << std::endl;
            return simulationPlayer;
        } catch (const ValidationException &exception) {
            autoFix.dumpInfo(this->logger);
            return nullptr;
        }
    } else {
        std::unique_ptr<RamtrjPlayer> player;
        try {
            player = std::make_unique<RamtrjPlayer>(std::move(trajectoryStream));
        } catch (const ValidationException &exception) {
            this->logger.error() << "Reading the trajectory failed: " << exception.what() << std::endl;
            this->logger << "You may try to fix it by adding the --auto-fix option." << std::endl;
            return nullptr;
        }
        Validate(player->getNumMolecules() == numMolecules);
        return player;
    }
}

void Frontend::createWalls(Packing &packing, const std::array<bool, 3> &walls) {
    for (std::size_t i{}; i < 3; i++)
        packing.toggleWall(i, walls[i]);
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
    std::string shape;
    std::vector<std::string> outputs;

    // TODO: documentation for --output
    options.add_options()
        ("h,help", "prints help for this mode")
        ("i,input", "an INI/PYON file with parameters of the shape; it can be used instead of manually "
                    "specifying shape parameters using -S",
         cxxopts::value<std::string>(inputFilename))
        ("S,shape", "manually specified shape (instead of reading from input file using -i); ",
         cxxopts::value<std::string>(shape))
        ("l,log-info", "prints information about the shape")
        ("o,output", "stores preview of the shape in a format given as an argument: wolfram, obj; multiple formats may "
                     "be passed using multiple -o options or separated by a pipe '|' in a single one",
         cxxopts::value<std::vector<std::string>>(outputs));

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    // Create shape traits
    std::shared_ptr<ShapeTraits> traits;
    if (parsedOptions.count("input")) {
        if (parsedOptions.count("shape"))
            die("Options -i (--input), -S (--shape) cannot be specified together", this->logger);

        RampackParameters params = this->dispatchParams(inputFilename);
        traits = params.baseParameters.shapeTraits;
    } else {
        if (!parsedOptions.count("shape")) {
            die("You must specify INI file with shape parameters using -i (--input) or do it manually using -S "
                "(--shape)", this->logger);
        }

        traits = this->createShapeTraits(shape);
    }

    if (!parsedOptions.count("log-info") && outputs.empty())
        die("At least one of options: -l (--log-info), -o (--output) must be specified", this->logger);

    // Log info
    if (parsedOptions.count("log-info")) {
        this->logger.info() << "Shape specification : " << shape << std::endl;
        this->logger << std::endl;
        this->printInteractionInfo(traits->getInteraction());
        this->logger << std::endl;
        this->printGeometryInfo(traits->getGeometry());
    }

    // Preview
    for (const auto &output : outputs) {
        auto printer = Frontend::createShapePrinter(output, *traits);
        printer.store(Shape{}, this->logger);
    }

    return EXIT_SUCCESS;
}

void Frontend::printGeometryInfo(const ShapeGeometry &geometry) {
    this->logger << "## Geometry info" << std::endl;
    this->logger << "Volume           : " << geometry.getVolume() << std::endl;
    this->logger << "Geometric origin : " << geometry.getGeometricOrigin({}) << std::endl;

    // Axes
    try {
        auto axis = geometry.getPrimaryAxis({});
        this->logger << "Primary axis     : " << axis << std::endl;
    } catch (std::runtime_error &) {
        this->logger << "Primary axis     : UNSPECIFIED" << std::endl;
    }
    try {
        auto axis = geometry.getSecondaryAxis({});
        this->logger << "Secondary axis   : " << axis << std::endl;
    } catch (std::runtime_error &) {
        this->logger << "Secondary axis   : UNSPECIFIED" << std::endl;
    }
    try {
        auto axis = geometry.getAuxiliaryAxis({});
        this->logger << "Auxiliary axis   : " << axis << std::endl;
    } catch (std::runtime_error &) {
        this->logger << "Auxiliary axis   : UNSPECIFIED" << std::endl;
    }

    // Named points
    this->logger << "Named points     :" << std::endl;
    auto points = geometry.getNamedPoints();
    std::size_t maxLength = std::max_element(points.begin(), points.end(), [](const auto &p1, const auto &p2) {
        return p1.first.length() < p2.first.length();
    })->first.length();

    for (const auto &[name, point] : points) {
        this->logger << "    " << std::left << std::setw(maxLength) << name << " = " << point << std::endl;
    }
}

void Frontend::printInteractionInfo(const Interaction &interaction) {
    auto displayBool = [](bool b) { return b ? "true" : "false"; };
    this->logger << "## Interaction info" << std::endl;
    this->logger << "Has hard part            : " << displayBool(interaction.hasHardPart()) << std::endl;
    this->logger << "Has soft part            : " << displayBool(interaction.hasSoftPart()) << std::endl;
    this->logger << "Has wall part            : " << displayBool(interaction.hasWallPart()) << std::endl;
    this->logger << "Interaction center range : " << interaction.getRangeRadius() << std::endl;
    this->logger << "Total range              : " << interaction.getTotalRangeRadius() << std::endl;
    this->logger << "Interaction centers      :" << std::endl;
    auto interactionCentres = interaction.getInteractionCentres();
    if (interactionCentres.empty())
        interactionCentres.emplace_back();
    for (std::size_t i{}; i < interactionCentres.size(); i++)
        this->logger << "    [" << i << "] = " << interactionCentres[i] << std::endl;
}

void Frontend::attachSnapshotOut(ObservablesCollector &collector, const std::string &filename,
                                 bool isContinuation) const
{
    std::unique_ptr<std::ofstream> out;
    if (isContinuation)
        out = std::make_unique<std::ofstream>(filename, std::ios_base::app);
    else
        out = std::make_unique<std::ofstream>(filename);

    ValidateOpenedDesc(*out, filename, "to store observables");
    this->logger.info() << "Observable snapshots are stored on the fly to '" << filename << "'" << std::endl;
    collector.attachOnTheFlyOutput(std::move(out));
}

Simulation::Environment Frontend::recreateEnvironment(const RampackParameters &params,
                                                      const PackingLoader &loader) const
{
    // Parse initial environment (from the global section)
    auto env = params.baseParameters.baseEnvironment;

    std::size_t startRunIndex = loader.getStartRunIndex();
    if (loader.isContinuation()) {
        // If it is continuation, we need to replay and combine all previous environments from all previous runs
        // together with a continued run, and overwrite move step sizes at the end - this way we start with the same
        // step sizes that were before
        Assert(loader.isRestored());
        for (std::size_t i{}; i <= startRunIndex; i++)
            Frontend::combineEnvironment(env, params.runs[i]);
        this->overwriteMoveStepSizes(env, loader.getAuxInfo());
    } else {
        // If it is not a continuation, we replay environments of the runs BEFORE the starting point, then overwrite
        // step sizes (because this is where the last simulation ended) and AFTER it, we apply the new environment from
        // the starting run
        for (std::size_t i{}; i < startRunIndex; i++)
            Frontend::combineEnvironment(env, params.runs[i]);
        if (loader.isRestored())
            this->overwriteMoveStepSizes(env, loader.getAuxInfo());
        Frontend::combineEnvironment(env, params.runs[startRunIndex]);
    }

    ValidateMsg(env.isComplete(), "Some of parameters: pressure, temperature, moveTypes, scalingType are missing");

    return env;
}

Simulation::Environment Frontend::recreateRawEnvironment(const RampackParameters &params,
                                                         std::size_t startRunIndex) const
{
    Expects(startRunIndex < params.runs.size());
    auto env = params.baseParameters.baseEnvironment;
    for (std::size_t i{}; i <= startRunIndex; i++)
        Frontend::combineEnvironment(env, params.runs[i]);
    return env;
}

std::unique_ptr<Packing> Frontend::recreatePacking(PackingLoader &loader, const BaseParameters &params,
                                                   const ShapeTraits &traits, std::size_t maxThreads)
{
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    loader.loadPacking(std::move(bc), traits.getInteraction(), maxThreads, maxThreads);

    if (loader.isAllFinished())
        return nullptr;

    std::unique_ptr<Packing> packing;
    if (loader.isRestored()) {
        packing = loader.releasePacking();
    } else {
        // Same number of scaling and domain threads
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();
        packing = params.packingFactory->createPacking(std::move(pbc), traits, maxThreads, maxThreads);
    }

    this->createWalls(*packing, params.walls);

    return packing;
}

void Frontend::verifyDynamicParameter(const DynamicParameter &dynamicParameter, const std::string &parameterName,
                                      const IntegrationRun &run, std::size_t cycleOffset) const
{
    if (run.averagingCycles == 0)
        return;

    std::size_t firstAveragingCycle = run.thermalizationCycles.value_or(0) + cycleOffset;
    std::size_t totalCycles = firstAveragingCycle + run.averagingCycles.value_or(0);
    double constantValue = dynamicParameter.getValueForCycle(firstAveragingCycle, totalCycles);
    for (std::size_t averagingCycle = firstAveragingCycle + 1; averagingCycle < totalCycles; averagingCycle++) {
        double cycleValue = dynamicParameter.getValueForCycle(averagingCycle, totalCycles);
        if (cycleValue != constantValue) {
            // Calculate precision which is just large enough (plus some margin) to show the difference between values
            double relativeDiff = (cycleValue - constantValue)/std::max(std::abs(cycleValue), std::abs(constantValue));
            int minimalPrecision = static_cast<int>(-std::log10(std::abs(relativeDiff)));
            const int margin = 3;
            int precision = minimalPrecision + margin;
            this->logger << std::setprecision(precision);

            this->logger.error() << "Dynamic parameter '" << parameterName << "' is not constant in the averaging ";
            this->logger << "phase (for cycle >= " << firstAveragingCycle << ")." << std::endl;
            this->logger << "Namely, for cycles from " << firstAveragingCycle << " to " << (averagingCycle - 1) << " ";
            this->logger << "it is equal " << constantValue << ", but at cycle " << averagingCycle << " it changes to ";
            this->logger << cycleValue << std::endl;
            this->logger << std::setprecision(std::numeric_limits<double>::max_digits10);
            this->logger << "Use piecewise parameter and set it to constant value " << constantValue << " starting ";
            this->logger << "from cycle " << firstAveragingCycle << " or disable averaging phase." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

std::shared_ptr<ShapeTraits> Frontend::createShapeTraits(const std::string &shapeName) const {
    using namespace pyon;
    using namespace pyon::matcher;
    Any shapeTraits;
    auto shapeAST = Parser::parse(shapeName);
    auto matchReport = ShapeMatcher::shape.match(shapeAST, shapeTraits);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return shapeTraits.as<std::shared_ptr<ShapeTraits>>();
}

ObservablesMatcher::ObservableData Frontend::createObservable(const std::string &expression, std::size_t maxThreads) {
    using namespace pyon;
    using namespace pyon::matcher;

    auto observableMatcher = ObservablesMatcher::createObservablesMatcher(maxThreads);
    auto observableAST = Parser::parse(expression);
    Any observable;
    auto matchReport = observableMatcher.match(observableAST, observable);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return observable.as<ObservablesMatcher::ObservableData>();
}

std::shared_ptr<BulkObservable> Frontend::createBulkObservable(const std::string &expression, std::size_t maxThreads) {
    using namespace pyon;
    using namespace pyon::matcher;

    auto bulkObservableMatcher = ObservablesMatcher::createBulkObservablesMatcher(maxThreads);
    auto bulkObservableAST = Parser::parse(expression);
    Any bulkObservable;
    auto matchReport = bulkObservableMatcher.match(bulkObservableAST, bulkObservable);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return bulkObservable.as<std::shared_ptr<BulkObservable>>();
}

RampackParameters Frontend::dispatchParams(const std::string &filename) {
    std::ifstream file(filename);
    ValidateOpenedDesc(file, filename, "to load the simulation script");

    std::string firstLine;
    std::getline(file, firstLine);
    trim(firstLine);
    file.seekg(0, std::ios::beg);

    if (startsWith(firstLine, "rampack")) {
        this->logger.info() << "Parameters format in '" << filename << "' recognized as: PYON" << std::endl;
        return this->parsePyon(file);
    } else {
        this->logger.info() << "Parameters format in '" << filename << "' recognized as: INI" << std::endl;
        return this->parseIni(file);
    }
}

RampackParameters Frontend::parseIni(std::istream &in) {
    Parameters params(in);
    return IniParametersFactory::create(params);
}

RampackParameters Frontend::parsePyon(std::istream &in) {
    using namespace pyon;
    using namespace pyon::matcher;

    auto rampackMatcher = RampackMatcher::create();
    auto paramsAST = Parser::parse(in);
    Any params;
    auto matchReport = rampackMatcher.match(paramsAST, params);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return params.as<RampackParameters>();
}

FileSnapshotWriter Frontend::createFileSnapshotWriter(const std::string &expression) {
    using namespace pyon;
    using namespace pyon::matcher;

    auto writerMatcher = FileSnapshotWriterMatcher::create();
    auto writerAST = Parser::parse(expression);
    Any writer;
    auto matchReport = writerMatcher.match(writerAST, writer);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return writer.as<FileSnapshotWriter>();
}

FileShapePrinter Frontend::createShapePrinter(const std::string &expression, const ShapeTraits &traits) {
    using namespace pyon;
    using namespace pyon::matcher;

    auto printerMatcher = FileShapePrinterMatcher::create(traits);
    auto printerAST = Parser::parse(expression);
    Any printer;
    auto matchReport = printerMatcher.match(printerAST, printer);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return printer.as<FileShapePrinter>();
}

std::shared_ptr<SimulationRecorderFactory> Frontend::createSimulationRecorderFactory(const std::string &expression) {
    using namespace pyon;
    using namespace pyon::matcher;

    auto factoryMatcher = SimulationRecorderFactoryMatcher::create();
    auto factoryAST = Parser::parse(expression);
    Any factory;
    auto matchReport = factoryMatcher.match(factoryAST, factory);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return factory.as<std::shared_ptr<SimulationRecorderFactory>>();
}
