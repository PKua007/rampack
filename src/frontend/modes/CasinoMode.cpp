//
// Created by Piotr Kubala on 25/01/2023.
//

#include <iomanip>
#include <fstream>
#include <set>
#include <filesystem>

#include <cxxopts.hpp>

#include "CasinoMode.h"
#include "utils/Utils.h"
#include "core/shapes/CompoundShapeTraits.h"
#include "core/PeriodicBoundaryConditions.h"


int CasinoMode::main(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Monte Carlo sampling for both hard and soft potentials.");

    std::string inputFilename;
    std::string verbosity;
    std::string startFrom;
    std::size_t continuationCycles;
    std::string auxOutput;
    std::string auxVerbosity;

    options
        .set_width(120)
        .add_options()
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
            ModeBase::combineEnvironment(env, run);

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

void CasinoMode::performIntegration(Simulation &simulation, Simulation::Environment &env, const IntegrationRun &run,
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

void CasinoMode::performOverlapRelaxation(Simulation &simulation, Simulation::Environment &env,
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

Simulation::Environment CasinoMode::recreateEnvironment(const RampackParameters &params,
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
            ModeBase::combineEnvironment(env, params.runs[i]);
        this->overwriteMoveStepSizes(env, loader.getAuxInfo());
    } else {
        // If it is not a continuation, we replay environments of the runs BEFORE the starting point, then overwrite
        // step sizes (because this is where the last simulation ended) and AFTER it, we apply the new environment from
        // the starting run
        for (std::size_t i{}; i < startRunIndex; i++)
            ModeBase::combineEnvironment(env, params.runs[i]);
        if (loader.isRestored())
            this->overwriteMoveStepSizes(env, loader.getAuxInfo());
        ModeBase::combineEnvironment(env, params.runs[startRunIndex]);
    }

    ValidateMsg(env.isComplete(), "Some of parameters: pressure, temperature, moveTypes, scalingType are missing");

    return env;
}



std::unique_ptr<Packing> CasinoMode::recreatePacking(PackingLoader &loader, const BaseParameters &params,
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

    ModeBase::createWalls(*packing, params.walls);

    return packing;
}

void CasinoMode::verifyDynamicParameter(const DynamicParameter &dynamicParameter, const std::string &parameterName,
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

void CasinoMode::printPerformanceInfo(const Simulation &simulation) {
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

void CasinoMode::printAverageValues(const ObservablesCollector &collector) {
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

void CasinoMode::storeAverageValues(const std::string &filename, const ObservablesCollector &collector, double temperature,
                                    double pressure) const
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

void CasinoMode::overwriteMoveStepSizes(Simulation::Environment &env,
                                        const std::map<std::string, std::string> &packingAuxInfo) const
{
    std::set<std::string> notUsedStepSizes;
    for (const auto &[key, value] : packingAuxInfo)
        if (CasinoMode::isStepSizeKey(key))
            notUsedStepSizes.insert(key);

    for (auto &moveSampler : env.getMoveSamplers()) {
        auto groupName = moveSampler->getName();
        for (const auto &[moveName, stepSize] : moveSampler->getStepSizes()) {
            std::string moveKey = CasinoMode::formatMoveKey(groupName, moveName);
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
        std::string scalingKey = CasinoMode::formatMoveKey("scaling", "scaling");
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

void CasinoMode::attachSnapshotOut(ObservablesCollector &collector, const std::string &filename,
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

void CasinoMode::printMoveStatistics(const Simulation &simulation) const {
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

bool CasinoMode::isStepSizeKey(const std::string &key) {
    return startsWith(key, "step.");
}

std::string CasinoMode::formatMoveKey(const std::string &groupName, const std::string &moveName) {
    std::string moveKey = "step.";
    moveKey += groupName;
    moveKey += ".";
    moveKey += moveName;
    return moveKey;
}