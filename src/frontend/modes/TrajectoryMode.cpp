//
// Created by Piotr Kubala on 25/01/2023.
//

#include <chrono>

#include <cxxopts.hpp>

#include "TrajectoryMode.h"
#include "utils/Utils.h"
#include "frontend/RampackParameters.h"
#include "core/PeriodicBoundaryConditions.h"
#include "frontend/PackingLoader.h"
#include "core/SimulationPlayer.h"
#include "core/io/RamtrjPlayer.h"
#include "core/io/TruncatedPlayer.h"
#include "frontend/matchers/SimulationRecorderFactoryMatcher.h"
#include "frontend/matchers/ObservablesMatcher.h"
#include "frontend/matchers/FileSnapshotWriterMatcher.h"


int TrajectoryMode::main(int argc, char **argv) {
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

    options
        .set_width(120)
        .add_options()
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
                             "single one (-o 'obs1|obs2'). It is advisable to put the argument in '...' to escape shell "
                             "special characters '()\"\"|'",
             cxxopts::value<std::vector<std::string>>(observables))
            ("b,output-bulk-obs", "calculate bulk observables and output them to the file with a name given by the "
                                  "specified pattern. In the pattern, every occurrence of {} is replaced with observable "
                                  "signature name. If not specified, '_{}.txt' is appended at the end. Bulk observables "
                                  "are specified using -B (--bulk-observable)",
             cxxopts::value<std::string>(bulkObsOutputFilename))
            ("B,bulk-observable", "replays the simulation and calculates specified bulk observables (format as in the "
                                  "input file). Observables can be passed using multiple options (-o obs1 -o obs2) "
                                  "or pipe-separated in a single one (-o 'obs1|obs2'). It is advisable to put the argument "
                                  "in '...' to escape shell special characters '()\"\"|'",
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
    RampackParameters rampackParams = this->io.dispatchParams(inputFilename);
    const auto &baseParams = rampackParams.baseParameters;
    auto shapeTraits = baseParams.shapeTraits;
    auto packingFactory = baseParams.packingFactory;
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    auto packing = packingFactory->createPacking(std::move(pbc), *shapeTraits, maxThreads, maxThreads);
    packing->toggleWalls(baseParams.walls);

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
    std::unique_ptr<SimulationPlayer> player = this->io.loadRamtrjPlayer(trajectoryFilename, packing->size(), autoFix);
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
        auto factory = SimulationRecorderFactoryMatcher::match(trajectoryOutput);
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
            auto observableData = ObservablesMatcher::matchObservable(observable, maxThreads);
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
        this->io.storeSnapshots(collector, false, obsOutputFilename);
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
            auto theObservable = ObservablesMatcher::matchBulkObservable(bulkObservable, maxThreads);
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
        this->io.storeBulkObservables(collector, bulkObsOutputFilename);
        this->logger.info() << std::endl;
    }

    // Recreate RAMSNAP/XYZ/Wolfram file from the last snapshot (if desired)
    if (!snapshotOutputs.empty()) {
        this->logger.info() << "Reading last snapshot... " << std::flush;
        player->lastSnapshot(*packing, shapeTraits->getInteraction());
        this->logger.info() << "cycles: " << player->getCurrentSnapshotCycles() << std::endl;

        for (const auto &snapshotOutput: snapshotOutputs) {
            auto writer = FileSnapshotWriterMatcher::match(snapshotOutput);
            if (writer.getFilename() == trajectoryFilename) {
                die("Input trajectory file name '" + trajectoryFilename + "' cannot be used as an output!",
                    this->logger);
            }

            writer.generateSnapshot(*packing, *shapeTraits, player->getCurrentSnapshotCycles(), this->logger);
        }
    }

    return EXIT_SUCCESS;
}

Simulation::Environment TrajectoryMode::recreateRawEnvironment(const RampackParameters &params,
                                                               std::size_t startRunIndex) const
{
    Expects(startRunIndex < params.runs.size());
    auto env = params.baseParameters.baseEnvironment;
    for (std::size_t i{}; i <= startRunIndex; i++)
        combine_environment(env, params.runs[i]);
    return env;
}