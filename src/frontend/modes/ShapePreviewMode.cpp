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
    std::string shapeTraits;
    std::string shapePos;
    std::string shapeRot;
    std::string shapeParams;
    std::vector<std::string> outputs;

    options
        .set_width(120)
        .add_options()
            ("h,help", "prints help for this mode")
            ("i,input", "a PYON file with parameters of the shape; it can be used instead of manually "
                        "creating the shape using -S",
             cxxopts::value<std::string>(inputFilename))
            ("S,shape", "manually specified shape type (instead of reading from input file using `-i`). "
                        SHELL_SPECIAL_CHARACTERS_WARNING,
             cxxopts::value<std::string>(shapeTraits))
            ("p,shape-pos", "specific position of the shape as a 3-element PYON Array of Floats. "
                            SHELL_SPECIAL_CHARACTERS_WARNING,
             cxxopts::value<std::string>(shapePos)->default_value("[0, 0, 0]"))
            ("r,shape-rot", "specific orientation of the shape as a 3-element PYON array of Floats [XYZ Tait-Bryan "
                            "(aircraft) angles in degrees]. " SHELL_SPECIAL_CHARACTERS_WARNING,
             cxxopts::value<std::string>(shapeRot)->default_value("[0, 0, 0]"))
            ("d,shape-params", "shape params, which will be combined with the default values read from `--input` or "
                               "specified in `--shape`. It is a PYON dictionary, where key is param name and value is "
                               "Float, Integer, String, or a vector (3-element PYON Array of Floats), as accepted by "
                               "the shape type. " SHELL_SPECIAL_CHARACTERS_WARNING,
             cxxopts::value<std::string>(shapeParams)->default_value("{}"))
            ("l,log-info", "prints information about the shape")
            ("o,output", "stores preview of the shape in a format given as an argument: `wolfram`, `obj` "
                         "(Wavefront OBJ); multiple formats may be passed using multiple `-o` options or separated by "
                         "a pipe `|` in a single one. " SHELL_SPECIAL_CHARACTERS_WARNING,
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

        try {
            traits = ShapeMatcher::match(shapeTraits);
        } catch (const pyon::PyonException &e) {
            throw ValidationException(std::string("Error while parsing --shape:\n") + e.what());
        }
    }

    Shape trialShape = ShapePreviewMode::createTrialShape(shapePos, shapeRot, shapeParams, traits->getDataManager());

    if (!parsedOptions.count("log-info") && outputs.empty())
        throw ValidationException("At least one of options: -l (--log-info), -o (--output) must be specified");

    // Log info
    if (parsedOptions.count("log-info")) {
        this->logger.info() << "Shape specification : " << shapeTraits << std::endl;
        this->logger << std::endl;
        this->printInteractionInfo(traits->getInteraction(), trialShape);
        this->logger << std::endl;
        this->printGeometryInfo(traits->getGeometry(), trialShape);
    }

    // Preview
    for (const auto &output : outputs) {
        auto printer = FileShapePrinterMatcher::match(output, *traits);
        printer.store(trialShape, this->logger);
    }

    return EXIT_SUCCESS;
}

Shape ShapePreviewMode::createTrialShape(const std::string &shapePos, const std::string &shapeRot,
                                         const std::string &shapeParams, const ShapeDataManager &manager)
{
    Shape trialShape;

    try {
        trialShape.setPosition(ShapeMatcher::matchPosition(shapePos));
    } catch (const pyon::PyonException &e) {
        throw ValidationException(std::string("Error while parsing --shape-pos:\n") + e.what());
    }

    try {
        trialShape.setOrientation(ShapeMatcher::matchOrientation(shapeRot));
    } catch (const pyon::PyonException &e) {
        throw ValidationException(std::string("Error while parsing --shape-rot:\n") + e.what());
    }

    try {
        auto textualData = ShapeMatcher::matchShapeData(shapeParams);
        trialShape.setData(manager.defaultDeserialize(textualData));
    } catch (const pyon::PyonException &e) {
        throw ValidationException(std::string("Error while parsing --shape-params:\n") + e.what());
    } catch (const ShapeDataException &e) {
        throw ValidationException(std::string("Error while processing trial shape params:\n") + e.what());
    }
    return trialShape;
}

void ShapePreviewMode::printGeometryInfo(const ShapeGeometry &geometry, const Shape &trialShape) {
    this->logger << "## Geometry info" << std::endl;
    this->logger << "Volume           : " << geometry.getVolume(trialShape) << std::endl;
    Vector<3> absoluteOrigin = geometry.getGeometricOrigin(trialShape) + trialShape.getPosition();
    this->logger << "Geometric origin : " << absoluteOrigin << std::endl;

    // Axes
    try {
        auto axis = geometry.getPrimaryAxis(trialShape);
        this->logger << "Primary axis     : " << axis << std::endl;
    } catch (std::runtime_error &) {
        this->logger << "Primary axis     : UNSPECIFIED" << std::endl;
    }
    try {
        auto axis = geometry.getSecondaryAxis(trialShape);
        this->logger << "Secondary axis   : " << axis << std::endl;
    } catch (std::runtime_error &) {
        this->logger << "Secondary axis   : UNSPECIFIED" << std::endl;
    }
    try {
        auto axis = geometry.getAuxiliaryAxis(trialShape);
        this->logger << "Auxiliary axis   : " << axis << std::endl;
    } catch (std::runtime_error &) {
        this->logger << "Auxiliary axis   : UNSPECIFIED" << std::endl;
    }

    this->logger << "Named points     :" << std::endl;
    auto points = geometry.getNamedPoints();

    using FormattedPoint = std::pair<std::string, std::optional<Vector<3>>>;
    std::vector<FormattedPoint> formattedPoints;
    formattedPoints.reserve(points.size());
    int maxLength = 0;
    for (const auto &point : points) {
        maxLength = std::max(maxLength, static_cast<int>(point.getName().length()));
        if (point.isValidForShapeData(trialShape.getData()))
            formattedPoints.emplace_back(point.getName(), point.forShape(trialShape));
        else
            formattedPoints.emplace_back(point.getName(), std::nullopt);
    }

    for (const auto &[pointName, point] : formattedPoints) {
        this->logger << "    " << std::left << std::setw(maxLength) << pointName << " = ";
        if (point.has_value())
            this->logger << *point << std::endl;
        else
            this->logger << "UNDEFINED FOR THE SPECIFIC SHAPE" << std::endl;
    }
}

void ShapePreviewMode::printInteractionInfo(const Interaction &interaction, const Shape &trialShape) {
    auto rawData = trialShape.getData().raw();
    auto displayBool = [](bool b) { return b ? "true" : "false"; };
    this->logger << "## Interaction info" << std::endl;
    this->logger << "Has hard part            : " << displayBool(interaction.hasHardPart()) << std::endl;
    this->logger << "Has soft part            : " << displayBool(interaction.hasSoftPart()) << std::endl;
    this->logger << "Has wall part            : " << displayBool(interaction.hasWallPart()) << std::endl;
    this->logger << "Interaction center range : " << interaction.getRangeRadius(rawData) << std::endl;
    this->logger << "Total range              : " << interaction.getTotalRangeRadius(rawData) << std::endl;
    this->logger << "Interaction centers      :" << std::endl;
    auto interactionCentres = interaction.getInteractionCentres(rawData);
    if (interactionCentres.empty())
        interactionCentres.emplace_back();
    for (std::size_t i{}; i < interactionCentres.size(); i++) {
        Vector<3> absoluteInteractionCentre
            = trialShape.getOrientation()*interactionCentres[i] + trialShape.getPosition();
        this->logger << "    [" << i << "] = " << absoluteInteractionCentre << std::endl;
    }
}
