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

    options.add_options()
            ("h,help", "prints help for this mode")
            ("i,input", "an INI file with parameters. See input.ini for parameters description",
             cxxopts::value<std::string>(inputFilename))
            ("P,set_param", "overrides the value of the parameter loaded from --input parameter set. More precisely, "
                            "doing -P N=1 (-PN=1 does not work) acts as one would append N=1 to the input file",
             cxxopts::value<std::vector<std::string>>(overridenParams))
            ("V,verbosity", "how verbose the output should be. Allowed values, with increasing verbosity: "
                            "error, warn, info, verbose, debug",
             cxxopts::value<std::string>(verbosity)->default_value("info"));

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
    this->logger.info() << "Simulation parameters:" << std::endl;
    Parameters params = this->loadParameters(inputFilename, overridenParams);
    params.print(this->logger);
    this->logger << std::endl;

    // Prepare simulation
    double linearSize = std::cbrt(params.initialVolume);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    LatticeArrangingModel latticeArrangingModel;
    auto shapes = latticeArrangingModel.arrange(params.numOfParticles, linearSize);
    auto shapeTraits = ShapeFactory::shapeTraitsFor(params.shapeName, params.shapeAttributes, params.interaction);
    auto packing = std::make_unique<Packing>(linearSize, std::move(shapes), std::move(bc));
    Simulation simulation(params.temperature, params.pressure, params.positionStepSize, params.rotationStepSize,
                          params.volumeStepSize, params.thermalisationCycles, params.averagingCycles,
                          params.averagingEvery, params.seed);

    // Perform simulations
    simulation.perform(std::move(packing), shapeTraits->getInteraction(), logger);
    this->logger << "----------------------------" << std::endl;
    this->logger.info() << "Average density: " << simulation.getAverageDensity() << std::endl;
    this->logger << "----------------------------" << std::endl;
    this->logger << "Translation acceptance rate: " << simulation.getTranlationAcceptanceRate() << std::endl;
    this->logger << "Rotation acceptance rate: " << simulation.getRotationAcceptanceRate() << std::endl;
    this->logger << "Scaling acceptance rate: " << simulation.getScalingAcceptanceRate() << std::endl;
    this->logger << "----------------------------" << std::endl;

    // Store packing (if desired)
    if (!params.wolframFilename.empty()) {
        std::ofstream out(params.wolframFilename);
        ValidateMsg(out, "Could not open " + params.wolframFilename + " to store packing!");
        simulation.getPacking().toWolfram(out, shapeTraits->getPrinter());
        this->logger.info() << "Packing stored to " + params.wolframFilename << std::endl;
    }

    // Store density, energy, etc. (if desired)
    if (!params.outputFilename.empty()) {
        std::ofstream out(params.outputFilename, std::ios_base::app);
        ValidateMsg(out, "Could not open " + params.outputFilename + " to store output!");
        Quantity rho = simulation.getAverageDensity();
        Quantity E = simulation.getAverageEnergy();
        Quantity Efluct = simulation.getAverageEnergyFluctuations();
        double theta = rho.value * shapeTraits->getVolume();
        double Z = params.pressure / params.temperature / rho.value;
        out.precision(std::numeric_limits<double>::max_digits10);
        out << params.temperature << " " << params.pressure << " " << rho.value << " " << theta << " " << Z << " ";
        out << (E.value / params.numOfParticles) << " " << Efluct.value << std::endl;
        this->logger.info() << "Output factor stored to " + params.outputFilename << std::endl;
    }

    // Store density snapshots during thermalisation phase
    if (!params.densitySnapshotFilename.empty()) {
        std::ofstream out(params.densitySnapshotFilename);
        ValidateMsg(out, "Could not open " + params.densitySnapshotFilename + " to store density snapshots!");
        auto snapshots = simulation.getDensityThermalisationSnapshots();
        out.precision(std::numeric_limits<double>::max_digits10);
        std::copy(snapshots.begin(), snapshots.end(),
                  std::ostream_iterator<Simulation::ScalarSnapshot>(out, "\n"));
        this->logger.info() << "Density snapshots stored to " + params.densitySnapshotFilename << std::endl;
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
