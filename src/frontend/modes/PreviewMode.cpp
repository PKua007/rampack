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
    std::vector<std::string> outputs;

    options
        .set_width(120)
        .add_options()
            ("h,help", "prints help for this mode")
            ("i,input", "an INI/PYON file with parameters. See sample_inputs folder for full parameters documentation",
             cxxopts::value<std::string>(inputFilename))
            ("o,output", "output of the preview. Supported PYON classes: ramsnap, wolfram, xyz (see the input file "
                         "documentation). More than one format can be chosen by specifying this option multiple times, or "
                         "in a single one using pipe '|'. It is advisable to put the argument in '...' to escape shell "
                         "special characters '()\"\"|'",
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

    RampackParameters params = this->io.dispatchParams(inputFilename);
    const auto &baseParams = params.baseParameters;
    auto shapeTraits = baseParams.shapeTraits;
    auto packingFactory = baseParams.packingFactory;
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    auto packing = packingFactory->createPacking(std::move(pbc), *shapeTraits, 1, 1);
    ModeBase::createWalls(*packing, baseParams.walls);

    for (const auto &output : outputs) {
        auto writer = FileSnapshotWriterMatcher::match(output);
        writer.generateSnapshot(*packing, *shapeTraits, 0, this->logger);
    }

    return EXIT_SUCCESS;
}