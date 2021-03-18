//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cstdlib>
#include <fstream>
#include <limits>
#include <iterator>
#include <algorithm>

#include <cxxopts.hpp>

#include "Frontend.h"
#include "Parameters.h"
#include "utils/Fold.h"
#include "utils/Utils.h"
#include "utils/Assertions.h"
#include "ShapeFactory.h"
#include "core/Simulation.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/LatticeArrangingModel.h"
#include "core/Packing.h"
#include "utils/OMPMacros.h"


Parameters Frontend::loadParameters(const std::string &inputFilename, const std::vector<std::string> &overridenParams) {
    std::ifstream paramsFile(inputFilename);
    if (!paramsFile)
        die("Cannot open " + inputFilename + " to read parameters from.", logger);
    std::stringstream paramsStream;
    paramsStream << paramsFile.rdbuf() << std::endl;

    for (const auto &overridenParam : overridenParams) {
        std::size_t equalPos = overridenParam.find('=');
        if (equalPos == std::string::npos)
            die("Malformed overriden param. Use: [param name]=[value]",logger);

        paramsStream << overridenParam << std::endl;
    }

    return Parameters(paramsStream);
}

void Frontend::setOverridenParamsAsAdditionalText(std::vector<std::string> overridenParams) const {
    if (overridenParams.empty())
        return;

    for (auto &param : overridenParams) {
        std::regex pattern(R"([a-zA-Z0-9_]+\.(.+=.+))");
        std::string replaced = std::regex_replace(param, pattern, "$1");
        if (!replaced.empty())
            param = replaced;
    }

    std::ostringstream additionalText;
    std::copy(overridenParams.begin(), overridenParams.end() - 1,
              std::ostream_iterator<std::string>(additionalText, ", "));
    additionalText << overridenParams.back();
    this->logger.setAdditionalText(additionalText.str());
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
    cxxopts::Options options(argv[0], "Hard particle Monte Carlo.");

    std::string inputFilename;
    std::vector<std::string> overridenParams;
    std::string verbosity;
    std::string startFrom;
    std::size_t continuationCycles;

    options.add_options()
            ("h,help", "prints help for this mode")
            ("i,input", "an INI file with parameters. See input.ini for parameters description",
             cxxopts::value<std::string>(inputFilename))
            ("P,set-param", "overrides the value of the parameter loaded from --input parameter set. More precisely, "
                            "doing -P N=1 (-PN=1 does not work) acts as one would append N=1 to the input file",
             cxxopts::value<std::vector<std::string>>(overridenParams))
            ("V,verbosity", "how verbose the output should be. Allowed values, with increasing verbosity: "
                            "error, warn, info, verbose, debug",
             cxxopts::value<std::string>(verbosity)->default_value("info"))
            ("s,start-from", "when specified, the simulation will be started at the specified run from "
                             "the input file. The packing will be restored from the file specified by the preceding "
                             "run, or from the previous run of this step if used together with --continue option.",
             cxxopts::value<std::string>(startFrom))
            ("c,continue", "when specified, the previously generated packing will be loaded and continued "
                           "for as many more cycles as specified. It can be used together with --start-from. The "
                           "of continuation cycles overrides both the value from input file and --set-param.",
             cxxopts::value<std::size_t>(continuationCycles));

    auto parsedOptions = options.parse(argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    this->setOverridenParamsAsAdditionalText(overridenParams);
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

    Parameters params = this->loadParameters(inputFilename, overridenParams);
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

    std::size_t moveThreads = std::accumulate(domainDivisions.begin(), domainDivisions.end(), 1, std::multiplies<>{});
    Validate(moveThreads > 0);
    Validate(moveThreads <= static_cast<std::size_t>(_OMP_MAXTHREADS));

    // Info about threads
    this->logger << _OMP_MAXTHREADS << " OpenMP threads are available" << std::endl;
    this->logger << "Using " << scalingThreads << " for scaling" << std::endl;
    if (moveThreads == 1) {
        this->logger << "Using 1 thread without domain decomposition for moves" << std::endl;
    } else {
        this->logger << "Using " << moveThreads << " threads with " << domainDivisions[0] << " x ";
        this->logger << domainDivisions[1] << " x " << domainDivisions[2] << " domain division" << std::endl;
    }
    this->logger << "--------------------------------------------------------------------" << std::endl;

    // Find starting run index if specified
    std::size_t startRunIndex{};
    if (parsedOptions.count("start-from")) {
        auto runsParameters = params.runsParameters;
        auto nameMatchesStartFrom = [startFrom](const Parameters::RunParameters &params) {
            return params.runName == startFrom;
        };
        auto it = std::find_if(runsParameters.begin(), runsParameters.end(), nameMatchesStartFrom);

        ValidateMsg(it != runsParameters.end(), "Invalid run name to start from");
        startRunIndex = it - runsParameters.begin();
    }

    // Override thermalization cycles if this is continuation run
    if (parsedOptions.count("continue")) {
        Validate(continuationCycles > 0);
        auto &startingRun = params.runsParameters[startRunIndex];
        startingRun.thermalisationCycles = continuationCycles;
        this->logger.info() << "Thermalisation from the finished run '" << startingRun.runName << "' will be ";
        this->logger << "continued for " << continuationCycles << " more cycles" << std::endl;
    }

    // Load starting state from a previous or current run packing depending on --start-from and --continue
    // options combination
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    std::unique_ptr<Packing> packing;

    if ((parsedOptions.count("start-from") && startRunIndex != 0) || parsedOptions.count("continue")) {
        packing = std::make_unique<Packing>(std::move(bc), moveThreads, scalingThreads);
        auto runsParameters = params.runsParameters;
        Parameters::RunParameters startingRunParams;
        if (parsedOptions.count("continue"))
            startingRunParams = runsParameters[startRunIndex];
        else
            startingRunParams = runsParameters[startRunIndex - 1];

        std::string previousPackingFilename = startingRunParams.packingFilename;
        std::ifstream packingFile(previousPackingFilename);
        ValidateMsg(packingFile, "Cannot open file " + previousPackingFilename + " to restore starting state");
        packing->restore(packingFile, shapeTraits->getInteraction());

        std::string previousRunName = startingRunParams.runName;
        this->logger.info() << "Loaded packing from the run '" << previousRunName << "' as a starting point.";
        this->logger << std::endl;
    }

    // If packing was not loaded from file, arrange it as given in config file
    if (packing == nullptr) {
        std::istringstream dimensionsStream(params.initialDimensions);
        std::array<double, 3> dimensions{};
        dimensionsStream >> dimensions[0] >> dimensions[1] >> dimensions[2];
        ValidateMsg(dimensionsStream, "Invalid packing dimensions format. Expected: [dim x] [dim y] [dim z]");
        Validate(std::all_of(dimensions.begin(), dimensions.end(), [](double d) { return d > 0; }));
        auto shapes = this->arrangePacking(params.numOfParticles, dimensions, params.initialArrangement);
        packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(bc), shapeTraits->getInteraction(),
                                            moveThreads, scalingThreads);
    }

    // Perform simulations starting from initial run
    Simulation simulation(std::move(packing), params.positionStepSize, params.rotationStepSize, params.volumeStepSize,
                          params.seed, domainDivisions);

    for (std::size_t i = startRunIndex; i < params.runsParameters.size(); i++) {
        auto runParams = params.runsParameters[i];

        this->logger.info() << std::endl;
        this->logger << "--------------------------------------------------------------------" << std::endl;
        this->logger << "Starting run '" << runParams.runName << "'" << std::endl;
        this->logger << "--------------------------------------------------------------------" << std::endl;
        runParams.print(logger);
        this->logger << "--------------------------------------------------------------------" << std::endl;

        simulation.perform(runParams.temperature, runParams.pressure, runParams.thermalisationCycles,
                           runParams.averagingCycles, runParams.averagingEvery, shapeTraits->getInteraction(), logger);

        // Print info
        Quantity rho = simulation.getAverageDensity();
        Quantity E = simulation.getAverageEnergy();
        E.value = E.value / params.numOfParticles;
        E.error = E.error / params.numOfParticles;
        Quantity Efluct = simulation.getAverageEnergyFluctuations();
        Quantity theta(rho.value * shapeTraits->getVolume(), rho.error * shapeTraits->getVolume());
        theta.separator = Quantity::PLUS_MINUS;
        Quantity Z(runParams.pressure / runParams.temperature / rho.value,
                   runParams.pressure / runParams.temperature * rho.error / rho.value / rho.value);
        Z.separator = Quantity::PLUS_MINUS;

        std::size_t ngRebuilds = simulation.getPacking().getNeighbourGridRebuilds();
        std::size_t ngResizes = simulation.getPacking().getNeighbourGridResizes();
        double ngRebuildSeconds = simulation.getPacking().getNeighbourGridRebuildMicroseconds() / 1e6;
        double moveSeconds = simulation.getMoveMicroseconds() / 1e6;
        double scalingSeconds = simulation.getScalingMicroseconds() / 1e6;
        double totalSeconds = moveSeconds + scalingSeconds;
        double cyclesPerSecond = static_cast<double>(runParams.thermalisationCycles + runParams.averagingCycles)
                                 / totalSeconds;
        double ngRebuildTotalPercent = ngRebuildSeconds / totalSeconds * 100;
        double ngRebuildScalingPercent = ngRebuildSeconds / scalingSeconds * 100;
        double movePercent = moveSeconds / totalSeconds * 100;
        double scalingPercent = scalingSeconds / totalSeconds * 100;

        this->logger.info();
        this->logger << "--------------------------------------------------------------------" << std::endl;
        this->logger << "Average density                          : " << rho << std::endl;
        this->logger << "Average packing fraction                 : " << theta << std::endl;
        this->logger << "Average compressibility factor           : " << Z << std::endl;
        this->logger << "Average energy per particle              : " << E << std::endl;
        this->logger << "Average energy fluctuations per particle : " << Efluct << std::endl;
        this->logger << "--------------------------------------------------------------------" << std::endl;
        this->logger << "Move acceptance rate            : " << simulation.getMoveAcceptanceRate() << std::endl;
        this->logger << "Scaling acceptance rate         : " << simulation.getScalingAcceptanceRate() << std::endl;
        this->logger << "Neighbour grid resizes/rebuilds : " << ngResizes << "/" << ngRebuilds << std::endl;
        this->logger << "--------------------------------------------------------------------" << std::endl;
        this->logger << "Move time         : " << moveSeconds << " s (" << movePercent << "% total)" << std::endl;
        this->logger << "Scaling time      : " << scalingSeconds << " s (" << scalingPercent << "% total)" << std::endl;
        this->logger << "NG rebuild time   : " << ngRebuildSeconds << " s (";
        this->logger << ngRebuildScalingPercent << "% scaling, " << ngRebuildTotalPercent << "% total)" <<  std::endl;
        this->logger << "Total time        : " << totalSeconds << " s" << std::endl;
        this->logger << "Cycles per second : " << cyclesPerSecond << std::endl;
        this->logger << "--------------------------------------------------------------------" << std::endl;

        // Store packing (if desired)
        if (!runParams.packingFilename.empty()) {
            std::ofstream out(runParams.packingFilename);
            ValidateMsg(out, "Could not open " + runParams.packingFilename + " to store packing!");
            simulation.getPacking().store(out);
            this->logger.info() << "Packing stored to " + runParams.packingFilename << std::endl;
        }

        // Store Mathematica packing (if desired)
        if (!runParams.wolframFilename.empty()) {
            std::ofstream out(runParams.wolframFilename);
            ValidateMsg(out, "Could not open " + runParams.wolframFilename + " to store Wolfram packing!");
            simulation.getPacking().toWolfram(out, shapeTraits->getPrinter());
            this->logger.info() << "Wolfram packing stored to " + runParams.wolframFilename << std::endl;
        }

        // Store density, energy, etc. (if desired)
        if (!runParams.outputFilename.empty()) {
            std::ofstream out(runParams.outputFilename, std::ios_base::app);
            ValidateMsg(out, "Could not open " + runParams.outputFilename + " to store output!");
            out.precision(std::numeric_limits<double>::max_digits10);
            out << runParams.temperature << " " << runParams.pressure << " " << rho.value << " " << theta.value << " ";
            out << Z.value << " " << E.value << " " << Efluct.value << std::endl;
            this->logger.info() << "Output factor stored to " + runParams.outputFilename << std::endl;
        }

        // Store density snapshots during thermalisation phase
        if (!runParams.densitySnapshotFilename.empty()) {
            std::ofstream out(runParams.densitySnapshotFilename);
            ValidateMsg(out, "Could not open " + runParams.densitySnapshotFilename + " to store density snapshots!");
            auto snapshots = simulation.getDensityThermalisationSnapshots();
            out.precision(std::numeric_limits<double>::max_digits10);
            std::copy(snapshots.begin(), snapshots.end(),
                      std::ostream_iterator<Simulation::ScalarSnapshot>(out, "\n"));
            this->logger.info() << "Density snapshots stored to " + runParams.densitySnapshotFilename << std::endl;
        }
    }

    return EXIT_SUCCESS;
}

int Frontend::analyze([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    return EXIT_SUCCESS;
}

int Frontend::printGeneralHelp(const std::string &cmd) {
    std::ostream &rawOut = this->logger;

    rawOut << Fold("Random and Maximal PACKing PACKage - computational package dedicated to simulate various packing "
                   "models.").width(80) << std::endl;
    rawOut << std::endl;
    rawOut << "Usage: " << cmd << " [mode] (mode dependent parameters). " << std::endl;
    rawOut << std::endl;
    rawOut << "Available modules:" << std::endl;
    rawOut << "casino" << std::endl;
    rawOut << Fold("Hard particle Monte Carlo.").width(80).margin(4) << std::endl;
    rawOut << "analyze" << std::endl;
    rawOut << Fold("Statistical analysis of simulations.").width(80).margin(4) << std::endl;
    rawOut << std::endl;
    rawOut << "Type " + cmd + " [mode] --help to get help on the specific mode." << std::endl;

    return EXIT_SUCCESS;
}

std::vector<Shape> Frontend::arrangePacking(std::size_t numOfParticles, const std::array<double, 3> &boxDimensions,
                                            const std::string &arrangementString)
{
    std::istringstream arrangementStream(arrangementString);
    std::string type;
    arrangementStream >> type;
    ValidateMsg(arrangementStream, "Malformed arrangement. Usage: [type: now only lattice] "
                                   "(type dependent parameters)");
    if (type == "lattice") {
        LatticeArrangingModel model;
        if (arrangementStream.str().find("default") != std::string::npos) {
            std::string defaultStr;
            arrangementStream >> defaultStr;
            ValidateMsg(arrangementStream && defaultStr == "default",
                        "Malformed latice arrangement. Usage: lattice {default|[cell size x] [... y] [... z] "
                        "[number of particles in line x] [... y] [... z]}");
            return model.arrange(numOfParticles, boxDimensions);
        } else {
            std::array<double, 3> cellDimensions{};
            std::array<std::size_t, 3> particlesInLine{};
            arrangementStream >> cellDimensions[0] >> cellDimensions[1] >> cellDimensions[2];
            arrangementStream >> particlesInLine[0] >> particlesInLine[1] >> particlesInLine[2];
            ValidateMsg(arrangementStream,
                        "Malformed latice arrangement. Usage: lattice {default|[cell size x] [... y] [... z] "
                        "[number particles in line x] [... y] [... z]}");
            Validate(std::all_of(cellDimensions.begin(), cellDimensions.end(), [](double d) { return d; }));
            Validate(std::accumulate(particlesInLine.begin(), particlesInLine.end(), 1., std::multiplies<>{})
                     >= numOfParticles);
            return model.arrange(numOfParticles, particlesInLine, cellDimensions, boxDimensions);
        }
    }

    throw ValidationException("Unknown arrangement type: " + type + ". Available: now only lattice");
}
