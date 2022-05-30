//
// Created by pkua on 29.05.22.
//

#include <sstream>
#include <optional>
#include <algorithm>
#include <iterator>
#include <variant>

#include <ZipIterator.hpp>

#include "LatticeBuilder.h"
#include "core/TriclinicBox.h"
#include "utils/Utils.h"
#include "core/lattice/Lattice.h"
#include "core/lattice/LatticeTransformer.h"
#include "core/lattice/LatticePopulator.h"
#include "core/lattice/UnitCellFactory.h"
#include "core/lattice/RandomPopulator.h"
#include "core/lattice/SerialPopulator.h"


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


    template <typename T>
    std::vector<T> tokenize(std::istream &in) {
        std::vector<T> tokens;
        in >> std::ws;
        while (in.good()) {
            T token;
            in >> token;
            if (in.good())
                in >> std::ws;
            tokens.push_back(token);
        }
        return tokens;
    }

    template <typename T>
    std::vector<T> tokenize(const std::string &str) {
        std::istringstream in(str);
        return tokenize<T>(in);
    }

    std::optional<TriclinicBox> parse_box(std::string boxString) {
        trim(boxString);
        if (boxString == "auto")
            return std::nullopt;

        std::istringstream boxStream(boxString);
        auto tokens = tokenize<double>(boxStream);
        ValidateMsg(!boxStream.fail(), BOX_DIMENSIONS_USAGE);

        switch (tokens.size()) {
            case 1:
                ValidateMsg(tokens[0] > 0, "Box side length must be > 0");
                return TriclinicBox(tokens[0]);
            case 3:
                ValidateMsg(std::all_of(tokens.begin(), tokens.end(), [](auto d) { return d > 0; }),
                            "Box side lengths must be > 0");
                return TriclinicBox(std::array<double, 3>{tokens[0], tokens[1], tokens[2]});
            case 9:
                return TriclinicBox(Matrix<3, 3>{tokens[0], tokens[1], tokens[2],
                                                 tokens[3], tokens[4], tokens[5],
                                                 tokens[6], tokens[7], tokens[8]});
            default:
                throw ValidationException(BOX_DIMENSIONS_USAGE);
        }
    }

    CellDimensions parse_cell_dim(const std::string &dim) {
        std::istringstream boxStream(dim);
        auto tokens = tokenize<double>(boxStream);
        ValidateMsg(!boxStream.fail(), BOX_DIMENSIONS_USAGE);

        switch (tokens.size()) {
            case 1:
                ValidateMsg(tokens[0] > 0, "Cell linear size must be > 0");
                return tokens[0];
            case 3:
                ValidateMsg(std::all_of(tokens.begin(), tokens.end(), [](auto d) { return d > 0; }),
                            "Cell side lengths must be > 0");
                return std::array<double, 3>{tokens[0], tokens[1], tokens[2]};
            case 9:
                return TriclinicBox(Matrix<3, 3>{tokens[0], tokens[1], tokens[2],
                                                 tokens[3], tokens[4], tokens[5],
                                                 tokens[6], tokens[7], tokens[8]});
            default:
                throw ValidationException(CELL_DIMENSIONS_USAGE);
        }
    }

    std::map<std::string, std::string> parse_fields(const std::vector<std::string> &fields,
                                                    const std::vector<std::string> &tokens)
    {
        std::map<std::string, std::string> fieldMap;
        auto currField = fieldMap.end();
        for (const auto &token : tokens) {
            if (std::find(fields.begin(), fields.end(), token) != fields.end()) {
                if (fieldMap.find(token) != fieldMap.end())
                    throw ValidationException("Redefined field: " + token);
                currField = fieldMap.insert({token, ""}).first;
            } else {
                if (currField == fieldMap.end()) {
                    if (std::find(fields.begin(), fields.end(), "") == fields.end())
                        throw ValidationException("Empty field name is not supported in this context");
                    currField = fieldMap.insert({"", ""}).first;
                }

                auto &value = currField->second;
                if (value.empty()) {
                    value = token;
                } else {
                    value += " ";
                    value += token;
                }
            }
        }
        return fieldMap;
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
            auto tokens = tokenize<double>(shapeStream);
            ValidateMsg(!shapeStream.fail(), "Malformed shape. Usage: [pos. x] [y] [z] ([angle x deg] [y] [z])");
            constexpr double f = M_PI / 180;
            switch (tokens.size()) {
                case 3:
                    shapes.emplace_back(Vector<3>{tokens[0], tokens[1], tokens[2]});
                    break;
                case 6:
                    shapes.emplace_back(Vector<3>{tokens[0], tokens[1], tokens[2]},
                                        Matrix<3, 3>::rotation(f*tokens[3], f*tokens[4], f*tokens[5]));
                    break;
                default:
                    throw ValidationException("Malformed shape. Usage: [pos. x] [y] [z] ([angle x deg] [y] [z])");
            }
        }
        return shapes;
    }

    std::array<std::size_t, 3> parse_ncell(const std::string &ncellString) {
        std::istringstream dimStream(ncellString);
        auto latticeDimTokens = tokenize<std::size_t>(dimStream);
        ValidateMsg(!dimStream.fail() && latticeDimTokens.size() == 3, "Malformed 'ncell'");
        auto greaterThanZero = [](auto dim) { return dim > 0; };
        ValidateMsg(std::all_of(latticeDimTokens.begin(), latticeDimTokens.end(), greaterThanZero),
                    "All 'ncell' elements have to be > 0");
        std::array<std::size_t, 3> latticeDim{};
        std::copy(latticeDimTokens.begin(), latticeDimTokens.end(), latticeDim.begin());
        return latticeDim;
    }

    Lattice prepare_lattice(std::size_t numParticles, std::optional<TriclinicBox> requestedBox,
                            const std::string &cellDefinition)
    {
        auto tokens = tokenize<std::string>(cellDefinition);
        std::string cellType = tokens.front();
        tokens.erase(tokens.begin());

        std::map<std::string, std::string> fieldMap;
        std::optional<UnitCell> cell;
        if (cellType == "sc") {
            fieldMap = parse_fields({"ncell", "dim", "default"}, tokens);
            cell = std::visit([](const auto &&arg) { return UnitCellFactory::createScCell(arg); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "bcc") {
            fieldMap = parse_fields({"ncell", "dim", "default"}, tokens);
            cell = std::visit([](const auto &&arg) { return UnitCellFactory::createBccCell(arg); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "fcc") {
            fieldMap = parse_fields({"ncell", "dim", "default"}, tokens);
            cell = std::visit([](const auto &&arg) { return UnitCellFactory::createFccCell(arg); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "hcp") {
            fieldMap = parse_fields({"ncell", "dim", "default", "axis"}, tokens);
            LatticeTraits::Axis axis = LatticeTraits::Axis::Z;
            if (fieldMap.find("axis") != fieldMap.end())
                axis = parse_axis(fieldMap["axis"]);
            cell = std::visit([axis](const auto &&arg) { return UnitCellFactory::createHcpCell(arg, axis); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "hexagonal") {
            fieldMap = parse_fields({"ncell", "dim", "default", "axis"}, tokens);
            LatticeTraits::Axis axis = LatticeTraits::Axis::Z;
            if (fieldMap.find("axis") != fieldMap.end())
                axis = parse_axis(fieldMap["axis"]);
            cell = std::visit([axis](const auto &&arg) { return UnitCellFactory::createHexagonalCell(arg, axis); },
                              parse_cell_dim(value_or_default(fieldMap, "dim", "1")));
        } else if (cellType == "custom") {
            fieldMap = parse_fields({"ncell", "dim", "default", "shapes"}, tokens);
            if (fieldMap.find("shapes") == fieldMap.end())
                throw ValidationException("Shapes have to be specified for the custom unit cell");
            auto shapes = parse_shapes(fieldMap["shapes"]);
            cell = UnitCell(std::visit([](const auto &&arg) { return TriclinicBox(arg); },
                                       parse_cell_dim(value_or_default(fieldMap, "dim", "1"))),
                            shapes);
        } else {
            throw ValidationException("Unknown cell type: " + cellType);
        }

        std::array<std::size_t, 3> latticeDim{};

        if (fieldMap.find("dim") == fieldMap.end()) {
            ValidateMsg(requestedBox.has_value(),
                        "Automatic box size not supported if either of: 'dim', 'ncell' is not specified");
            if (fieldMap.find("ncell") == fieldMap.end()) {
                ValidateMsg(fieldMap.find("default") != fieldMap.end(),
                            "If 'ncell' field not present, 'default' should be specified");
                ValidateMsg(fieldMap["default"].empty(), "Unexpected token: " + fieldMap["default"]);

                double allCells = std::ceil(static_cast<double>(numParticles) / static_cast<double>(cell->size()));
                auto ncell = static_cast<std::size_t>(std::ceil(std::cbrt(allCells)));
                latticeDim = {ncell, ncell, ncell};
            } else {
                ValidateMsg(fieldMap.find("default") == fieldMap.end(),
                            "'default' cannot be specified together with 'ncell'");
                latticeDim = parse_ncell(fieldMap["ncell"]);
            }

            auto cellSides = requestedBox->getSides();
            std::transform(cellSides.begin(), cellSides.end(), latticeDim.begin(), cellSides.begin(),
                           [](const auto &cellSide, auto dim) { return cellSide / static_cast<double>(dim); });
            cell->getBox() = TriclinicBox(cellSides);
        } else {
            ValidateMsg(!requestedBox.has_value(), "If explicit cell size is specified, box size should be 'auto'");
            ValidateMsg(fieldMap.find("ncell") != fieldMap.end(), "'ncell' must be specified together with 'dim'");
            ValidateMsg(fieldMap.find("default") == fieldMap.end(), "'default' cannot be specified together with 'dim'");
            latticeDim = parse_ncell(fieldMap["ncell"]);
        }

        return Lattice(*cell, latticeDim);
    }

    auto prepare_operations(const std::vector<std::string> &latticeOperations, const Interaction &interaction,
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

                std::string populatorType;
                operationStream >> populatorType;
                ValidateMsg(operationStream, "Populator type has to be specified: serial, random");

                if (populatorType == "random") {
                    unsigned long seed{};
                    operationStream >> seed;
                    ValidateMsg(operationStream, "Malformed random populator. Usage: populate random [rng seed]");
                    populator = std::make_unique<RandomPopulator>(seed);
                } else if (populatorType == "serial") {
                    std::string axisOrder;
                    operationStream >> axisOrder;
                    if (!operationStream)
                        axisOrder = "xyz";

                    try {
                        populator = std::make_unique<SerialPopulator>(axisOrder);
                    } catch (const LatticeTraits::AxisOrderParseException &) {
                        throw ValidationException("Malformed serial populator axis order. Usage: populate serial "
                                                  "[axis order: xyz, yxz, etc.]");
                    }
                } else {
                    throw ValidationException("Unknown populator type: " + populatorType + ". Use: serial, random");
                }
            } else {
                ValidateMsg(populator == nullptr, "Cannot apply further transformations after populating the lattice");

                throw ValidationException("not implemented");
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

    auto lattice = prepare_lattice(numParticles, requestedBox, cellDefinition);
    auto [transformers, populator] = prepare_operations(latticeOperations, interaction, geometry);

    for (const auto &transformer : transformers)
        transformer->transform(lattice);
    auto shapes = populator->populateLattice(lattice, numParticles);
    auto latticeBox = lattice.getLatticeBox();

    return std::make_unique<Packing>(latticeBox, shapes, std::move(bc), interaction, moveThreads, scalingThreads);
}
