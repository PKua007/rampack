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
#include "core/arranging_models/OrthorombicArrangingModel.h"
#include "core/Packing.h"
#include "core/volume_scalers/DeltaVolumeScaler.h"
#include "core/volume_scalers/LinearVolumeScaler.h"
#include "core/volume_scalers/LogVolumeScaler.h"
#include "utils/OMPMacros.h"
#include "utils/Utils.h"
#include "core/MinimalDistanceOptimizer.h"


Parameters Frontend::loadParameters(const std::string &inputFilename, const std::vector<std::string> &overridenParams) {
    std::ifstream paramsFile(inputFilename);
    ValidateOpenedDesc(paramsFile, inputFilename, "to load input parameters");
    std::stringstream paramsStream;
    paramsStream << paramsFile.rdbuf() << std::endl;

    for (const auto &overridenParam : overridenParams) {
        std::size_t equalPos = overridenParam.find('=');
        if (equalPos == std::string::npos)
            die("Malformed overriden param. Use: [param name]=[value]", this->logger);

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
    std::unique_ptr<VolumeScaler> volumeScaler = this->createVolumeScaler(params.scalingType);

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
        Parameters::RunParameters &startingPackingRun = runsParameters[startingPackingRunIndex];

        std::string previousPackingFilename = startingPackingRun.packingFilename;
        std::ifstream packingFile(previousPackingFilename);
        ValidateOpenedDesc(packingFile, previousPackingFilename, "to load previous packing");
        // Same number of scaling and domain decemposition threads
        packing = std::make_unique<Packing>(std::move(bc), scalingThreads, scalingThreads);
        auto auxInfo = packing->restore(packingFile, shapeTraits->getInteraction());

        params.positionStepSize = std::stod(auxInfo.at("translationStep"));
        params.rotationStepSize = std::stod(auxInfo.at("rotationStep"));
        params.volumeStepSize = std::stod(auxInfo.at("scalingStep"));
        Validate(params.positionStepSize > 0);
        Validate(params.rotationStepSize > 0);
        Validate(params.volumeStepSize > 0);

        if (parsedOptions.count("continue")) {
            cycleOffset = std::stoul(auxInfo.at("cycles"));
            isContinuation = true;
            Validate(continuationCycles > 0);
            Validate(continuationCycles > cycleOffset);
            auto &startingRun = startingPackingRun;     // Because we continue this already finished run
            startingRun.thermalisationCycles = continuationCycles - cycleOffset;
            this->logger.info() << "Thermalisation from the finished run '" << startingRun.runName;
            this->logger << "' will be continued up to " << continuationCycles << " cycles (";
            this->logger << startingRun.thermalisationCycles << " to go)" << std::endl;
        }

        std::string previousRunName = startingPackingRun.runName;
        this->logger.info() << "Loaded packing from the run '" << previousRunName << "' as a starting point.";
        this->logger << std::endl;
    }

    // If packing was not loaded from file, arrange it as given in config file
    if (packing == nullptr) {
        std::array<double, 3> dimensions = this->parseDimensions(params.initialDimensions);
        // Same number of scaling and domain threads
        packing = this->arrangePacking(params.numOfParticles, dimensions, params.initialArrangement, std::move(bc),
                                       shapeTraits->getInteraction(), scalingThreads, scalingThreads);
    }

    // Perform simulations starting from initial run
    Simulation simulation(std::move(packing), params.positionStepSize, params.rotationStepSize, params.volumeStepSize,
                          params.seed, std::move(volumeScaler), domainDivisions, params.saveOnSignal);

    for (std::size_t i = startRunIndex; i < params.runsParameters.size(); i++) {
        auto runParams = params.runsParameters[i];

        this->logger.setAdditionalText(runParams.runName);
        this->logger.info() << std::endl;
        this->logger << "--------------------------------------------------------------------" << std::endl;
        this->logger << "Starting run '" << runParams.runName << "'" << std::endl;
        this->logger << "--------------------------------------------------------------------" << std::endl;
        runParams.print(logger);
        this->logger << "--------------------------------------------------------------------" << std::endl;

        auto collector = ObservablesCollectorFactory::create(explode(runParams.observables, ','));

        auto start = std::chrono::high_resolution_clock::now();
        simulation.perform(runParams.temperature, runParams.pressure, runParams.thermalisationCycles,
                           runParams.averagingCycles, runParams.averagingEvery, runParams.snapshotEvery,
                           *shapeTraits, std::move(collector), this->logger, cycleOffset);
        auto end = std::chrono::high_resolution_clock::now();
        double totalSeconds = std::chrono::duration<double>(end - start).count();

        // Print info
        const ObservablesCollector &observablesCollector = simulation.getObservablesCollector();

        const auto &simulatedPacking = simulation.getPacking();
        std::size_t ngRebuilds = simulatedPacking.getNeighbourGridRebuilds();
        std::size_t ngResizes = simulatedPacking.getNeighbourGridResizes();

        double ngRebuildSeconds = simulatedPacking.getNeighbourGridRebuildMicroseconds() / 1e6;
        double moveSeconds = simulation.getMoveMicroseconds() / 1e6;
        double scalingSeconds = simulation.getScalingMicroseconds() / 1e6;
        double observablesSeconds = simulation.getObservablesMicroseconds() / 1e6;
        double otherSeconds = totalSeconds - moveSeconds - scalingSeconds - observablesSeconds;
        double cyclesPerSecond = static_cast<double>(simulation.getPerformedCycles()) / totalSeconds;

        double ngRebuildTotalPercent = ngRebuildSeconds / totalSeconds * 100;
        double ngRebuildScalingPercent = ngRebuildSeconds / scalingSeconds * 100;
        double movePercent = moveSeconds / totalSeconds * 100;
        double scalingPercent = scalingSeconds / totalSeconds * 100;
        double observablesPercent = observablesSeconds / totalSeconds * 100;
        double otherPercent = otherSeconds / totalSeconds * 100;

        this->logger.info();
        this->logger << "--------------------------------------------------------------------" << std::endl;
        this->printAverageValues(observablesCollector);
        this->logger << "--------------------------------------------------------------------" << std::endl;
        this->logger << "Move acceptance rate            : " << simulation.getMoveAcceptanceRate() << std::endl;
        this->logger << "Scaling acceptance rate         : " << simulation.getScalingAcceptanceRate() << std::endl;
        this->logger << "Neighbour grid resizes/rebuilds : " << ngResizes << "/" << ngRebuilds << std::endl;
        this->logger << "Average neighbours per centre   : " << simulatedPacking.getAverageNumberOfNeighbours();
        this->logger << std::endl;
        this->logger << "--------------------------------------------------------------------" << std::endl;
        this->logger << "Move time         : " << moveSeconds << " s (" << movePercent << "% total)" << std::endl;
        this->logger << "Scaling time      : " << scalingSeconds << " s (" << scalingPercent << "% total)" << std::endl;
        this->logger << "NG rebuild time   : " << ngRebuildSeconds << " s (";
        this->logger << ngRebuildScalingPercent << "% scaling, " << ngRebuildTotalPercent << "% total)" <<  std::endl;
        this->logger << "Observables time  : " << observablesSeconds << " s (";
        this->logger << observablesPercent << "% total)" << std::endl;
        this->logger << "Other time        : " << otherSeconds << " s (" << otherPercent << "% total)" << std::endl;
        this->logger << "Total time        : " << totalSeconds << " s" << std::endl;
        this->logger << "Cycles per second : " << cyclesPerSecond << std::endl;
        this->logger << "--------------------------------------------------------------------" << std::endl;

        // Store packing (if desired)
        if (!runParams.packingFilename.empty()) {
            std::map<std::string, std::string> auxInfo;
            auxInfo["translationStep"] = std::to_string(simulation.getCurrentTranslationStep());
            auxInfo["rotationStep"] = std::to_string(simulation.getCurrentRotationStep());
            auxInfo["scalingStep"] = std::to_string(simulation.getCurrentScalingStep());
            auxInfo["cycles"] = std::to_string(simulation.getTotalCycles());

            std::ofstream out(runParams.packingFilename);
            ValidateOpenedDesc(out, runParams.packingFilename, "to store packing data");
            simulation.getPacking().store(out, auxInfo);
            this->logger.info() << "Packing stored to " + runParams.packingFilename << std::endl;
        }

        // Store Mathematica packing (if desired)
        if (!runParams.wolframFilename.empty()) {
            std::ofstream out(runParams.wolframFilename);
            ValidateOpenedDesc(out, runParams.wolframFilename, "to store Wolfram packing");
            simulation.getPacking().toWolfram(out, shapeTraits->getPrinter());
            this->logger.info() << "Wolfram packing stored to " + runParams.wolframFilename << std::endl;
        }

        // Store average values of observables (if desired)
        if (!runParams.outputFilename.empty()) {
            this->storeAverageValues(runParams.outputFilename, observablesCollector, runParams.temperature,
                                     runParams.pressure);
            this->logger.info() << "Average values stored to " + runParams.outputFilename << std::endl;
        }

        // Store observables vs cycles snapshots (if desired)
        if (!runParams.densitySnapshotFilename.empty()) {
            if (isContinuation) {
                std::ofstream out(runParams.densitySnapshotFilename, std::ios_base::app);
                ValidateOpenedDesc(out, runParams.densitySnapshotFilename, "to store observables");
                observablesCollector.printSnapshots(out, false);
            } else {
                std::ofstream out(runParams.densitySnapshotFilename);
                ValidateOpenedDesc(out, runParams.densitySnapshotFilename, "to store observables");
                observablesCollector.printSnapshots(out, true);
            }

            this->logger.info() << "Density snapshots stored to " + runParams.densitySnapshotFilename << std::endl;
        }

        isContinuation = false;
        cycleOffset = 0;
        
        if (simulation.wasInterrupted())
            break;
    }

    return EXIT_SUCCESS;
}

std::unique_ptr<VolumeScaler> Frontend::createVolumeScaler(std::string scalingType) const {
    std::string independentString = "independent ";
    bool scaleTogether = !startsWith(scalingType, independentString);
    if (!scaleTogether)
        scalingType = scalingType.substr(independentString.length());

    using ScalingDirection = VolumeScaler::ScalingDirection;

    // Old delta V scaling
    if (scalingType == "delta V")
        return std::make_unique<DeltaVolumeScaler>();
    // Linear scaling
    else if (scalingType == "linear isotropic")
        return std::make_unique<LinearVolumeScaler>(ScalingDirection::ISOTROPIC);
    else if (scalingType == "linear anisotropic x")
        return std::make_unique<LinearVolumeScaler>(ScalingDirection::ANISOTROPIC_X);
    else if (scalingType == "linear anisotropic y")
        return std::make_unique<LinearVolumeScaler>(ScalingDirection::ANISOTROPIC_Y);
    else if (scalingType == "linear anisotropic z")
        return std::make_unique<LinearVolumeScaler>(ScalingDirection::ANISOTROPIC_Z);
    else if (scalingType == "linear anisotropic xyz")
        return std::make_unique<LinearVolumeScaler>(ScalingDirection::ANISOTROPIC_XYZ);
    // Log scaling
    else if (scalingType == "log isotropic")
        return std::make_unique<LogVolumeScaler>(ScalingDirection::ISOTROPIC, scaleTogether);
    else if (scalingType == "log anisotropic x")
        return std::make_unique<LogVolumeScaler>(ScalingDirection::ANISOTROPIC_X, scaleTogether);
    else if (scalingType == "log anisotropic y")
        return std::make_unique<LogVolumeScaler>(ScalingDirection::ANISOTROPIC_Y, scaleTogether);
    else if (scalingType == "log anisotropic z")
        return std::make_unique<LogVolumeScaler>(ScalingDirection::ANISOTROPIC_Z, scaleTogether);
    else if (scalingType == "log anisotropic xyz")
        return std::make_unique<LogVolumeScaler>(ScalingDirection::ANISOTROPIC_XYZ, scaleTogether);
    else
        throw ValidationException("Unknown scaling type: " + scalingType);
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
    rawOut << "optimize-distance" << std::endl;
    rawOut << Fold("Find minimal distances between shapes in given direction(s).")
              .width(80).margin(4) << std::endl;
    rawOut << "preview" << std::endl;
    rawOut << Fold("Based on the input file generate initial configuration and store Wolfram and/or *.dat packing.")
              .width(80).margin(4) << std::endl;
    rawOut << std::endl;
    rawOut << "Type " + cmd + " [mode] --help to get help on the specific mode." << std::endl;

    return EXIT_SUCCESS;
}

std::unique_ptr<Packing> Frontend::arrangePacking(std::size_t numOfParticles,
                                                  std::array<double, 3> boxDimensions,
                                                  const std::string &arrangementString,
                                                  std::unique_ptr<BoundaryConditions> bc,
                                                  const Interaction &interaction, std::size_t moveThreads,
                                                  std::size_t scalingThreads)
{
    std::istringstream arrangementStream(arrangementString);
    std::string type;
    arrangementStream >> type;
    ValidateMsg(arrangementStream, "Malformed arrangement. Usage: [type: orthorombic, presimulated] "
                                   "(type dependent parameters)");
    if (type == "orthorombic" || type == "lattice") {
        auto shapes = this->arrangeOrthorombicShapes(numOfParticles, boxDimensions, interaction,arrangementStream);
        return std::make_unique<Packing>(boxDimensions, std::move(shapes), std::move(bc), interaction, moveThreads,
                                         scalingThreads);
    } else if (type == "presimulated") {
        std::string filename;
        arrangementStream >> filename;
        ValidateMsg(arrangementStream, "Malformed presimulated arrangement. Usage: presimulated [packing dat file]");

        std::ifstream packingFile(filename);
        ValidateOpenedDesc(packingFile, filename, "to load initial configuration");

        auto packing = std::make_unique<Packing>(std::move(bc), moveThreads, scalingThreads);
        packing->restore(packingFile, interaction);
        return packing;
    } else {
        throw ValidationException("Unknown arrangement type: " + type + ". Available: orthorombic, presimulated");
    }
}

auto Frontend::parseAntipolar(std::istringstream &arrangementStream) const {
    bool antipolar = false;
    auto axis = OrthorombicArrangingModel::PolarAxis::X;
    if (arrangementStream.str().find("antipolar") != std::string::npos) {
        std::string antipolarStr;
        std::string axisStr;
        arrangementStream >> antipolarStr >> axisStr;
        ValidateMsg(arrangementStream && antipolarStr == "antipolar",
                    "Malformed latice arrangement. Usage: orthorombic (antipolar {x|y|z}) "
                    "{default|[cell size x] [... y] [... z] [number of particles in line x] [... y] [... z]}");
        antipolar = true;
        if (axisStr == "x")
            axis = OrthorombicArrangingModel::PolarAxis::X;
        else if (axisStr == "y")
            axis = OrthorombicArrangingModel::PolarAxis::Y;
        else if (axisStr == "z")
            axis = OrthorombicArrangingModel::PolarAxis::Z;
        else
            throw ValidationException("Only x, y, z axes are allowed for antipolar orthorombic arrangement");
    }

    return std::make_pair(antipolar, axis);
}

std::vector<Shape> Frontend::arrangeOrthorombicShapes(std::size_t numOfParticles, std::array<double, 3> &boxDimensions,
                                                      const Interaction &interaction,
                                                      std::istringstream &arrangementStream) const
{
    auto [isAntipolar, axis] = this->parseAntipolar(arrangementStream);

    OrthorombicArrangingModel model(isAntipolar, axis);
    if (arrangementStream.str().find("default") != std::string::npos) {
        std::string defaultStr;
        arrangementStream >> defaultStr;
        ValidateMsg(arrangementStream && defaultStr == "default",
                    "Malformed latice arrangement. Usage: orthorombic {default|[cell size x] [... y] [... z] "
                    "[number of particles in line x] [... y] [... z]}");
        ValidateMsg((boxDimensions != std::array<double, 3>{0, 0, 0}),
                    "Default arrangement unsupported for automatic box size");

        return model.arrange(numOfParticles, boxDimensions);
    } else if (arrangementStream.str().find("spacing") != std::string::npos) {
        std::string spacingStr;
        double spacing;
        std::array<std::size_t, 3> particlesInLine{};
        arrangementStream >> spacingStr >> spacing;
        arrangementStream >> particlesInLine[0] >> particlesInLine[1] >> particlesInLine[2];
        ValidateMsg(arrangementStream && spacingStr == "spacing",
                    "Malformed latice arrangement. Usage: orthorombic {default|[cell size x] [... y] [... z] "
                    "[number of particles in line x] [... y] [... z]}");
        Validate(spacing > 0);
        Validate(std::accumulate(particlesInLine.begin(), particlesInLine.end(), 1., std::multiplies<>{})
                 >= numOfParticles);

        Shape s1, s2;
        auto distances = MinimalDistanceOptimizer::forAxes(s1, s2, interaction);
        if (isAntipolar) {
            std::size_t axisNum = OrthorombicArrangingModel::getAxisNumber(axis);

            std::array<double, 3> angles{};
            angles.fill(0);
            angles[axisNum] = M_PI;

            Vector<3> direction;
            direction[axisNum] = 1;

            s2.rotate(Matrix<3, 3>::rotation(angles[0], angles[1], angles[2]));
            double antipolarDistance1 = MinimalDistanceOptimizer::forDirection(s1, s2, direction, interaction);
            double antipolarDistance2 = MinimalDistanceOptimizer::forDirection(s1, s2, -direction, interaction);
            distances[axisNum] = std::max(antipolarDistance1, antipolarDistance2);
        }

        std::array<double, 3> cellDimensions{};
        std::transform(distances.begin(), distances.end(), cellDimensions.begin(),
                       [spacing](double distance) { return distance + spacing; });
        std::transform(cellDimensions.begin(), cellDimensions.end(), particlesInLine.begin(), boxDimensions.begin(),
                       std::multiplies<>{});
        return model.arrange(numOfParticles, particlesInLine, cellDimensions, boxDimensions);
    } else {
        std::array<double, 3> cellDimensions{};
        std::array<std::size_t, 3> particlesInLine{};
        arrangementStream >> cellDimensions[0] >> cellDimensions[1] >> cellDimensions[2];
        arrangementStream >> particlesInLine[0] >> particlesInLine[1] >> particlesInLine[2];
        ValidateMsg(arrangementStream,
                    "Malformed latice arrangement. Usage: orthorombic {default|[cell size x] [... y] [... z] "
                    "[number particles in line x] [... y] [... z]}");
        Validate(std::all_of(cellDimensions.begin(), cellDimensions.end(), [](double d) { return d; }));
        Validate(std::accumulate(particlesInLine.begin(), particlesInLine.end(), 1., std::multiplies<>{})
                 >= numOfParticles);

        if (boxDimensions == std::array<double, 3>{0, 0, 0}) {
            std::transform(cellDimensions.begin(), cellDimensions.end(), particlesInLine.begin(),
                           boxDimensions.begin(), std::multiplies<>{});
        }

        return model.arrange(numOfParticles, particlesInLine, cellDimensions, boxDimensions);
    }
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
        ("i,input", "loads shape parameters from INI file with parameters. If not specified, --shape-name "
                    "must be specified manually",
         cxxopts::value<std::string>(inputFilename))
        ("s,shape-name", "if specified, overrides shape name from --input", cxxopts::value<std::string>(shapeName))
        ("a,shape-attributes", "if specified, overrides shape attributes from --input. If not specified and no "
                               "--input is passed, it defaults to the empty string",
         cxxopts::value<std::string>(shapeAttributes))
        ("I,interaction", "if specified, overrides interaction from --input. If not specified and and no --input is "
                          "passed, it defaults to the empty string",
         cxxopts::value<std::string>(interaction))
        ("1,rotation-1", "[x angle] [y angle] [z angle] - the external Euler angles in degrees to rotate the 1st shape",
         cxxopts::value<std::string>(rotation1Str)->default_value("0 0 0"))
        ("2,rotation-2", "[x angle] [y angle] [z angle] - the external Euler angles in degrees to rotate the 2nd shape",
         cxxopts::value<std::string>(rotation2Str)->default_value("0 0 0"))
        ("d,direction", "[x] [y] [z] - if specified, the minimal distance will be computed in the given directionsStr(s)",
         cxxopts::value<std::vector<std::string>>(directionsStr))
        ("A,axes", "if specified, the distance will be computed for x, y and z axes")
        ("m,minimal-output", "output only distances (and errors)");

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
        Parameters params = this->loadParameters(inputFilename, {});
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
        minimalDistanceStream << MinimalDistanceOptimizer::forDirection(shape1, shape2, direction,
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
    std::vector<std::string> overridenParams;

    options.add_options()
        ("h,help", "prints help for this mode")
        ("i,input", "an INI file with parameters. See input.ini for parameters description",
         cxxopts::value<std::string>(inputFilename))
        ("P,set-param", "overrides the value of the parameter loaded from --input parameter set. More precisely, "
                        "doing -P N=1 (-PN=1 does not work) acts as one would append N=1 to the input file",
         cxxopts::value<std::vector<std::string>>(overridenParams))
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

    Parameters params = this->loadParameters(inputFilename, overridenParams);
    std::array<double, 3> dimensions = this->parseDimensions(params.initialDimensions);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    auto shapeTraits = ShapeFactory::shapeTraitsFor(params.shapeName, params.shapeAttributes, params.interaction);
    auto packing = this->arrangePacking(params.numOfParticles, dimensions, params.initialArrangement, std::move(bc),
                                        shapeTraits->getInteraction(), 1, 1);

    // Store packing (if desired)
    if (parsedOptions.count("dat")) {
        std::map<std::string, std::string> auxInfo;
        auxInfo["translationStep"] = std::to_string(params.positionStepSize);
        auxInfo["rotationStep"] = std::to_string(params.rotationStepSize);
        auxInfo["scalingStep"] = std::to_string(params.volumeStepSize);
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
