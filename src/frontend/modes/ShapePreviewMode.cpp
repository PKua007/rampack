//
// Created by Piotr Kubala on 25/01/2023.
//

#include <iomanip>

#include <cxxopts.hpp>

#include "ShapePreviewMode.h"
#include "core/ShapeTraits.h"
#include "frontend/RampackParameters.h"
#include "utils/Utils.h"
#include "frontend/matchers/ShapeMatcher.h"
#include "frontend/matchers/FileShapePrinterMatcher.h"


int ShapePreviewMode::main(int argc, char **argv) {
    // Prepare and parse options
    cxxopts::Options options(argv[0], "Information and preview for the shape.");

    std::string inputFilename;
    std::string shape;
    std::vector<std::string> outputs;

    options
        .set_width(120)
        .add_options()
            ("h,help", "prints help for this mode")
            ("i,input", "a PYON file with parameters of the shape; it can be used instead of manually "
                        "creating the shape using -S",
             cxxopts::value<std::string>(inputFilename))
            ("S,shape", "manually specified shape (instead of reading from input file using `-i`). It is advisable to "
                        "put the argument in single quotes `' '` to escape special shell characters `\"()|`",
             cxxopts::value<std::string>(shape))
            ("l,log-info", "prints information about the shape")
            ("o,output", "stores preview of the shape in a format given as an argument: `wolfram`, `obj` "
                         "(Wavefront OBJ); multiple formats may be passed using multiple `-o` options or separated by "
                         "a pipe `|` in a single one. It is advisable to put the argument in `' '` to escape special "
                         "shell characters `\"()|`",
             cxxopts::value<std::vector<std::string>>(outputs));

    auto parsedOptions = ModeBase::parseOptions(options, argc, argv);
    if (parsedOptions.count("help")) {
        std::ostream &rawOut = this->logger;
        rawOut << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    // Create shape traits
    std::shared_ptr<ShapeTraits> traits;
    if (parsedOptions.count("input")) {
        if (parsedOptions.count("shape"))
            throw ValidationException("Options -i (--input), -S (--shape) cannot be specified together");

        RampackParameters params = this->io.dispatchParams(inputFilename);
        traits = params.baseParameters.shapeTraits;
    } else {
        if (!parsedOptions.count("shape")) {
            throw ValidationException("You must specify INI file with shape parameters using -i (--input) or do it "
                                      "manually using -S (--shape)");
        }

        traits = ShapeMatcher::match(shape);
    }

    if (!parsedOptions.count("log-info") && outputs.empty())
        throw ValidationException("At least one of options: -l (--log-info), -o (--output) must be specified");

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
        auto printer = FileShapePrinterMatcher::match(output, *traits);
        printer.store(Shape{}, this->logger);
    }

    return EXIT_SUCCESS;
}

void ShapePreviewMode::printGeometryInfo(const ShapeGeometry &geometry) {
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

void ShapePreviewMode::printInteractionInfo(const Interaction &interaction) {
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
