//
// Created by Piotr Kubala on 25/01/2023.
//

#include <cxxopts.hpp>

#include "PreviewMode.h"
#include "frontend/RampackParameters.h"
#include "core/PeriodicBoundaryConditions.h"
#include "frontend/matchers/FileSnapshotWriterMatcher.h"


int PreviewMode::main(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Initial arrangement preview.");

    std::string inputFilename;
    std::string verbosity;
    std::vector<std::string> outputs;

    options
        .set_width(120)
        .add_options()
            ("h,help", "prints help for this mode")
            ("i,input", "a PYON file with parameters. See "
                        "https://github.com/PKua007/rampack/blob/main/docs/input-file.md for the documentation of the "
                        "input file",
             cxxopts::value<std::string>(inputFilename))
            ("V,verbosity", "how verbose the output should be. Allowed values, with increasing verbosity: "
                            "`fatal`, `error`, `warn`, `info`, `verbose`, `debug`. Defaults to: `info`",
             cxxopts::value<std::string>(verbosity))
            ("o,output", "outputs the initial configuration loaded from the input file. Supported formats: "
                         "`ramsnap`, `wolfram`, `xyz`. More than one format can be chosen by specifying this option "
                         "multiple times, or in a single one using pipe `|`. " SHELL_SPECIAL_CHARACTERS_WARNING,
             cxxopts::value<std::vector<std::string>>(outputs))
            ("r,run-names", "output run names from the input file to the standard output. Use with "
                            "`-V warn` for a clean output");

    auto parsedOptions = ModeBase::parseOptions(options, argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    std::optional<std::string> verbosityOptional;
    if (parsedOptions.count("verbosity"))
        verbosityOptional = verbosity;
    this->setVerbosityLevel(verbosityOptional, std::nullopt, std::nullopt);

    // Validate parsed options
    std::string cmd(argv[0]);
    if (!parsedOptions.unmatched().empty())
        throw ValidationException("Unexpected positional arguments. See " + cmd + " --help");
    if (!parsedOptions.count("input"))
        throw ValidationException("Input file must be specified with option -i [input file name]");
    if (outputs.empty() && !parsedOptions.count("run-names"))
        throw ValidationException("At least one of options must be specified: -o (--output), -r (--run-names)");

    RampackParameters params = this->io.dispatchParams(inputFilename);

    if (parsedOptions.count("run-names")) {
        this->logger.info() << "Run names present in '" << inputFilename << "':" << std::endl;
        std::ostream &loggerRaw = this->logger.raw();
        for (const auto &run : params.runs)
            loggerRaw << RunBase::of(run).runName << std::endl;
    }

    const auto &baseParams = params.baseParameters;
    const auto &shapeTraits = *baseParams.shapeTraits;
    const auto &packingFactory = *baseParams.packingFactory;
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    auto packing = packingFactory.createPacking(std::move(pbc), shapeTraits, 1, 1);
    packing->toggleWalls(baseParams.walls);

    for (const auto &output : outputs) {
        auto writer = FileSnapshotWriterMatcher::match(output);
        writer.generateSnapshot(*packing, shapeTraits, 0, this->logger);
    }

    return EXIT_SUCCESS;
}