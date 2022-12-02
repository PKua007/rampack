//
// Created by pkua on 29.05.22.
//

#include <sstream>
#include <optional>
#include <algorithm>
#include <iterator>
#include <variant>


#include "utils/ParseUtils.h"
#include "LatticeBuilder.h"
#include "core/TriclinicBox.h"
#include "utils/Utils.h"
#include "core/lattice/Lattice.h"
#include "core/lattice/LatticeTransformer.h"
#include "core/lattice/LatticePopulator.h"
#include "core/lattice/UnitCellFactory.h"
#include "core/lattice/RandomPopulator.h"
#include "core/lattice/SerialPopulator.h"
#include "core/lattice/CellOptimizationTransformer.h"
#include "core/lattice/ColumnarTransformer.h"
#include "core/lattice/FlipRandomizingTransformer.h"
#include "core/lattice/LayerRotationTransformer.h"
#include "core/lattice/LayerWiseCellOptimizationTransformer.h"


#define BOX_DIMENSIONS_USAGE "Malformed box dimensions. Usage alternatives: \n" \
                             "1. auto \n" \
                             "2. [cube side length] \n" \
                             "3. [cuboid side length x] [... y] [... z] \n" \
                             "4. [box matrix row 1, col 1] [... 1 2] [... 1 3] [... 2 1] ... [... 3 3] \n"

#define CELL_DIMENSIONS_USAGE "Malformed cell dimensions. Usage alternatives: \n" \
                              "1. [linear size] \n" \
                              "2. [cuboid side length x] [... y] [... z] \n" \
                              "3. [box matrix row 1, col 1] [... 1 2] [... 1 3] [... 2 1] ... [... 3 3] \n"

namespace {
    using CellDimensions = std::variant<double, std::array<double, 3>, TriclinicBox>;

    std::optional<TriclinicBox> parse_box(std::string boxString) {
        trim(boxString);
        if (boxString == "auto")
            return std::nullopt;

        std::istringstream boxStream(boxString);
        auto tokens = ParseUtils::tokenize<double>(boxStream);
        ValidateMsg(boxStream, BOX_DIMENSIONS_USAGE);

        switch (tokens.size()) {
            case 1:
                ValidateMsg(tokens[0] > 0, "Box side length must be > 0");
                return TriclinicBox(tokens[0]);
            case 3:
                ValidateMsg(std::all_of(tokens.begin(), tokens.end(), [](auto d) { return d > 0; }),
                            "Box side lengths must be > 0");
                return TriclinicBox(std::array<double, 3>{tokens[0], tokens[1], tokens[2]});
            case 9:
                return TriclinicBox(Matrix<3, 3>{tokens[0], tokens[3], tokens[6],
                                                 tokens[1], tokens[4], tokens[7],
                                                 tokens[2], tokens[5], tokens[8]});
            default:
                throw ValidationException(BOX_DIMENSIONS_USAGE);
        }
    }

    CellDimensions parse_cell_dim(const std::string &dim) {
        std::istringstream boxStream(dim);
        auto tokens = ParseUtils::tokenize<double>(boxStream);
        ValidateMsg(boxStream, BOX_DIMENSIONS_USAGE);

        switch (tokens.size()) {
            case 1:
                ValidateMsg(tokens[0] > 0, "Cell linear size must be > 0");
                return tokens[0];
            case 3:
                ValidateMsg(std::all_of(tokens.begin(), tokens.end(), [](auto d) { return d > 0; }),
                            "Cell side lengths must be > 0");
                return std::array<double, 3>{tokens[0], tokens[1], tokens[2]};
            case 9:
                return TriclinicBox(Matrix<3, 3>{tokens[0], tokens[3], tokens[6],
                                                 tokens[1], tokens[4], tokens[7],
                                                 tokens[2], tokens[5], tokens[8]});
            default:
                throw ValidationException(CELL_DIMENSIONS_USAGE);
        }
    }

    std::string value_or_default(const std::map<std::string, std::string> &map, const std::string &key,
                                 const std::string &defaultValue)
    {
        auto it = map.find(key);
        if (it == map.end())
            return defaultValue;
        else
            return it->second;
    }

    LatticeTraits::Axis parse_axis(const std::string &axis) {
        if (axis == "x")
            return LatticeTraits::Axis::X;
        else if (axis == "y")
            return LatticeTraits::Axis::Y;
        else if (axis == "z")
            return LatticeTraits::Axis::Z;
        else
            throw ValidationException("Incorrect axis: " + axis);
    }

    std::vector<Shape> parse_shapes(const std::string &shapesString) {
        std::vector<std::string> shapesExploded = explode(shapesString, ',');
        ValidateMsg(!shapesExploded.empty(), "Shapes have to be specified for the custom unit cell");
        std::vector<Shape> shapes;
        shapes.reserve(shapesExploded.size());
        for (const auto &shapeString : shapesExploded) {
            std::istringstream shapeStream(shapeString);
            auto tokens = ParseUtils::tokenize<double>(shapeStream);
            ValidateMsg(shapeStream, "Malformed shape. Usage: [pos. x] [y] [z] ([angle x deg] [y] [z])");
            constexpr double toRad = M_PI / 180;
            switch (tokens.size()) {
                case 3:
                    shapes.emplace_back(Vector<3>{tokens[0], tokens[1], tokens[2]});
                    break;
                case 6:
                    shapes.emplace_back(Vector<3>{tokens[0], tokens[1], tokens[2]},
                                        Matrix<3, 3>::rotation(toRad*tokens[3], toRad*tokens[4], toRad*tokens[5]));
                    break;
                default:
                    throw ValidationException("Malformed shape. Usage: [pos. x] [y] [z] ([angle x deg] [y] [z])");
            }
        }
        return shapes;
    }

    std::array<std::size_t, 3> parse_lattice_dim(const std::string &ncellString) {
        std::istringstream dimStream(ncellString);
        auto latticeDimTokens = ParseUtils::tokenize<std::size_t>(dimStream);
        ValidateMsg(dimStream && latticeDimTokens.size() == 3, "Malformed 'ncell'");
        auto greaterThanZero = [](auto dim) { return dim > 0; };
        ValidateMsg(std::all_of(latticeDimTokens.begin(), latticeDimTokens.end(), greaterThanZero),
                    "All 'ncell' elements have to be > 0");
        std::array<std::size_t, 3> latticeDim{};
        std::copy(latticeDimTokens.begin(), latticeDimTokens.end(), latticeDim.begin());
        return latticeDim;
    }

    UnitCell parse_unit_cell(const std::string &cellType, const std::map<std::string, std::string> &fieldMap) {
        if (cellType == "sc") {
            return std::visit([](const auto &&arg) { return UnitCellFactory::createScCell(arg); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "bcc") {
            return std::visit([](const auto &&arg) { return UnitCellFactory::createBccCell(arg); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "fcc") {
            return std::visit([](const auto &&arg) { return UnitCellFactory::createFccCell(arg); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "hcp") {
            LatticeTraits::Axis axis = LatticeTraits::Axis::Z;
            if (fieldMap.find("axis") != fieldMap.end())
                axis = parse_axis(fieldMap.at("axis"));
            return std::visit([axis](const auto &&arg) { return UnitCellFactory::createHcpCell(arg, axis); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "hexagonal") {
            LatticeTraits::Axis axis = LatticeTraits::Axis::Z;
            if (fieldMap.find("axis") != fieldMap.end())
                axis = parse_axis(fieldMap.at("axis"));
            return std::visit([axis](const auto &&arg) { return UnitCellFactory::createHexagonalCell(arg, axis); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "custom") {
            if (fieldMap.find("shapes") == fieldMap.end())
                throw ValidationException("Shapes have to be specified for the custom unit cell");
            auto shapes = parse_shapes(fieldMap.at("shapes"));
            auto box = std::visit([](const auto &&arg) { return TriclinicBox(arg); },
                                  parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
            return UnitCell(box, shapes);
        } else {
            throw ValidationException("Unknown cell type: " + cellType);
        }
    }

    auto parse_explicit_lattice_spec(std::optional<TriclinicBox> requestedBox, const UnitCell &cell,
                                     const std::map<std::string, std::string> &fieldMap)
    {
        ValidateMsg(!requestedBox.has_value(), "If explicit cell size is specified, box size should be 'auto'");
        ValidateMsg(fieldMap.find("ncell") != fieldMap.end(), "'ncell' must be specified together with 'dim'");
        ValidateMsg(fieldMap.find("default") == fieldMap.end(),
                    "'default' cannot be specified together with 'dim'");
        return std::make_pair(parse_lattice_dim(fieldMap.at("ncell")), cell.getBox());
    }

    auto calculate_automatic_lattice_spec(std::size_t numParticles, const TriclinicBox &requestedBox,
                                          const UnitCell &cell, const std::map<std::string, std::string> &fieldMap)
    {
        std::array<std::size_t, 3> latticeDim{};
        if (fieldMap.find("ncell") == fieldMap.end()) {
            // User didn't specify number of cells - number of cells will be calculated in such a way that the unit cell
            // will be as cubic as possible (with similar heights)
            ValidateMsg(fieldMap.find("default") != fieldMap.end(),
                        "If 'ncell' field not present, 'default' should be specified");
            ValidateMsg(fieldMap.at("default").empty(), "Unexpected token: " + fieldMap.at("default"));

            std::size_t allCells;
            if (numParticles % cell.size() == 0)
                allCells = numParticles / cell.size();
            else
                allCells = numParticles / cell.size() + 1;

            auto heights = requestedBox.getHeights();
            double pseudoVolume = std::accumulate(heights.begin(), heights.end(), 1., std::multiplies<>{});
            double targetCellSize = std::cbrt(pseudoVolume / static_cast<double>(allCells));

            // Find the best integer number of cells
            auto dimCalculator = [targetCellSize](double height) {
                double bestNumCells = height/targetCellSize;
                return static_cast<std::size_t>(std::round(bestNumCells));
            };
            std::transform(heights.begin(), heights.end(), latticeDim.begin(), dimCalculator);

            // Increase number of cells along the longest side if there are too few cells to fit all particles
            while (std::accumulate(latticeDim.begin(), latticeDim.end(), 1ul, std::multiplies<>{}) < allCells) {
                auto maxDim = std::max_element(latticeDim.begin(), latticeDim.end());
                (*maxDim)++;
            }
        } else {
            // User specified explicitly number of cells - cell dimensions are calculated based on box dimensions
            ValidateMsg(fieldMap.find("default") == fieldMap.end(),
                        "'default' cannot be specified together with 'ncell'");
            latticeDim = parse_lattice_dim(fieldMap.at("ncell"));
        }

        auto cellSides = requestedBox.getSides();
        std::transform(cellSides.begin(), cellSides.end(), latticeDim.begin(), cellSides.begin(),
                       [](const auto &cellSide, auto dim) { return cellSide / static_cast<double>(dim); });
        return std::make_pair(latticeDim, TriclinicBox(cellSides));
    }

    auto parse_lattice_dim(std::size_t numParticles, std::optional<TriclinicBox> requestedBox, const UnitCell &cell,
                           const std::map<std::string, std::string> &fieldMap)
    {
        if (fieldMap.find("dim") != fieldMap.end()) {
            // Cell dim "dim" is explicitly specified, so the lattice size should also be specified explicitly, which
            // in the end gives full specification
            return parse_explicit_lattice_spec(requestedBox, cell, fieldMap);
        } else {
            // Cell dim "dim" is not specified explicitly, so it will be calculated automatically (based on box size
            // and/or lattice size)
            ValidateMsg(requestedBox.has_value(),
                        "Automatic box size not supported if either of: 'dim', 'ncell' is not specified");
            return calculate_automatic_lattice_spec(numParticles, *requestedBox, cell, fieldMap);
        }
    }

    Lattice parse_lattice(std::size_t numParticles, std::optional<TriclinicBox> requestedBox,
                          const std::string &cellDefinition)
    {
        auto tokens = ParseUtils::tokenize<std::string>(cellDefinition);
        std::string cellType = tokens.front();
        tokens.erase(tokens.begin());

        auto fieldMap = ParseUtils::parseFields({"ncell", "dim", "default", "axis", "shapes"}, tokens);
        auto cell = parse_unit_cell(cellType, fieldMap);
        auto [latticeDim, newCellBox] = parse_lattice_dim(numParticles, requestedBox, cell, fieldMap);
        cell.getBox() = newCellBox;

        return Lattice(cell, latticeDim);
    }

    std::unique_ptr<LatticePopulator> parse_populator(std::istringstream &operationStream) {
        std::string populatorType;
        operationStream >> populatorType;
        ValidateMsg(operationStream, "Populator type has to be specified: serial, random");

        if (populatorType == "random") {
            unsigned long seed{};
            operationStream >> seed;
            ValidateMsg(operationStream, "Malformed random populator. Usage: populate random [rng seed]");
            return std::make_unique<RandomPopulator>(seed);
        } else if (populatorType == "serial") {
            std::string axisOrder;
            operationStream >> axisOrder;
            if (!operationStream)
                axisOrder = "xyz";

            try {
                return std::make_unique<SerialPopulator>(axisOrder);
            } catch (const LatticeTraits::AxisOrderParseException &) {
                throw ValidationException("Malformed serial populator axis order. Usage: populate serial "
                                          "[axis order]");
            }
        } else {
            throw ValidationException("Unknown populator type: " + populatorType + ". Use: serial, random");
        }
    }

    std::unique_ptr<LatticeTransformer> parse_transformer(const std::string &operationType,
                                                          std::istringstream &operationStream,
                                                          const Interaction &interaction, const ShapeGeometry &geometry)
    {
        if (operationType == "optimizeCell") {
            double spacing{};
            std::string axisOrder;
            operationStream >> spacing >> axisOrder;
            ValidateMsg(operationStream, "Malformed transformation. Usage: optimizeCell [spacing] [axis order]");
            return std::make_unique<CellOptimizationTransformer>(interaction, axisOrder, spacing);
        } else if (operationType == "optimizeLayers") {
            double spacing{};
            std::string axisStr;
            operationStream >> spacing >> axisStr;
            LatticeTraits::Axis axis = parse_axis(axisStr);
            ValidateMsg(operationStream, "Malformed transformation. Usage: optimizeLayers [spacing] [axis]");
            return std::make_unique<LayerWiseCellOptimizationTransformer>(interaction, axis, spacing);
        } else if (operationType == "columnar") {
            std::string axisStr;
            unsigned long seed{};
            operationStream >> axisStr >> seed;
            ValidateMsg(operationStream, "Malformed transformation. Usage: columnar [column axis] [rng seed]");
            auto axis = parse_axis(axisStr);
            return std::make_unique<ColumnarTransformer>(axis, seed);
        } else if (operationType == "randomizeFlip") {
            unsigned long seed{};
            operationStream >> seed;
            ValidateMsg(operationStream, "Malformed transformation. Usage: randomizeFlip [rng seed]");
            return std::make_unique<FlipRandomizingTransformer>(geometry, seed);
        } else if (operationType == "layerRotate") {
            std::string layerAxisStr;
            std::string rotAxisStr;
            double angle;
            operationStream >> layerAxisStr >> rotAxisStr >> angle;
            ValidateMsg(operationStream, "Malformed transformation. Usage: layerRotate "
                                         "[layer axis] [rot. axis] [rot. angle] (alternating)");
            auto layerAxis = parse_axis(layerAxisStr);
            auto rotAxis = parse_axis(rotAxisStr);

            bool isAlternating = false;
            std::string alternatingStr;
            operationStream >> alternatingStr;
            if (operationStream) {
                ValidateMsg(alternatingStr == "alternating",
                            "Malformed transformation. Usage: layerRotate "
                            "[layer axis] [rot. axis] [rot. angle] (alternating)");
                isAlternating = true;
            }

            return std::make_unique<LayerRotationTransformer>(layerAxis, rotAxis, angle/180*M_PI, isAlternating);
        } else {
            throw ValidationException("Unknown transformation type: " + operationType + ". Supported: "
                                      + "optimizeCell, optimizeLayers, columnar, randomizeFlip, layerRotate");
        }
    }

    auto parse_operations(const std::vector<std::string> &latticeOperations, const Interaction &interaction,
                          const ShapeGeometry &geometry)
    {
        std::vector<std::unique_ptr<LatticeTransformer>> transformers;
        std::unique_ptr<LatticePopulator> populator;

        for (const auto &operation : latticeOperations) {
            std::istringstream operationStream(operation);
            std::string operationType;
            operationStream >> operationType;
            ValidateMsg(operationStream, "Lattice transformation cannot be empty");
            if (operationType == "populate") {
                ValidateMsg(populator == nullptr, "Redefinition of lattice populator type");
                populator = parse_populator(operationStream);
            } else {
                ValidateMsg(populator == nullptr, "Cannot apply further transformations after populating the lattice");
                auto trans = parse_transformer(operationType, operationStream, interaction, geometry);
                transformers.push_back(std::move(trans));
            }
        }

        if (populator == nullptr)
            populator = std::make_unique<SerialPopulator>("xyz");

        return std::make_pair(std::move(transformers), std::move(populator));
    }
}

std::vector<std::string> LatticeBuilder::getSupportedCellTypes() {
    return {"sc", "bcc", "fcc", "hcp", "hexagonal", "custom"};
}

std::unique_ptr<Packing> LatticeBuilder::buildPacking(std::size_t numParticles, const std::string &boxString,
                                                      const std::string &arrangementString,
                                                      std::unique_ptr<BoundaryConditions> bc,
                                                      const Interaction &interaction, const ShapeGeometry &geometry,
                                                      std::size_t moveThreads, std::size_t scalingThreads)
{
    auto requestedBox = parse_box(boxString);
    auto latticeOperations = explode(arrangementString, '|');
    ValidateMsg(!latticeOperations.empty(), "Initial arrangement cannot be empty");
    std::string cellDefinition = latticeOperations.front();
    latticeOperations.erase(latticeOperations.begin());

    auto lattice = parse_lattice(numParticles, requestedBox, cellDefinition);
    auto [transformers, populator] = parse_operations(latticeOperations, interaction, geometry);

    for (const auto &transformer : transformers)
        transformer->transform(lattice);
    lattice.normalize();
    auto shapes = populator->populateLattice(lattice, numParticles);
    auto latticeBox = lattice.getLatticeBox();

    return std::make_unique<Packing>(latticeBox, shapes, std::move(bc), interaction, moveThreads, scalingThreads);
}
