//
// Created by pkua on 01.12.2021.
//

#include <numeric>
#include <fstream>

#include "ArrangementFactory.h"
#include "core/lattice/OrthorhombicArrangingModel.h"
#include "core/lattice/DistanceOptimizer.h"
#include "utils/Assertions.h"
#include "core/PeriodicBoundaryConditions.h"
#include "LatticeBuilder.h"
#include "utils/ParseUtils.h"


#define ORTHORHOMBIC_USAGE "Malformed orthorhombic arrangement. Usage alternatives:\n" \
                           "1. orthorhombic ([clinicity]) ([polarization]) default\n" \
                           "2. orthorhombic ([clinicity]) ([polarization]) spacing [spacing value] [axis order] [num molecules x] [... y] [... z]\n" \
                           "3. orthorhombic ([clinicity]) ([polarization]) [num molecules x] [... y] [... z]\n" \
                           "4. orthorhombic ([clinicity]) ([polarization]) [cell size x] [... y] [... z] [num molecules x] [... y] [... z]\n" \
                           "[clinicity] := {synclinic|anticlinic} [tilt axis] [tilt angle]\n" \
                           "[polarization] := {polar|antipolar} [polarization axis]"

namespace {
    std::array<double, 3> parse_box_dimensions(const std::string &initialDimensions) {
        std::istringstream dimensionsStream(initialDimensions);
        std::array<double, 3> dimensions{};
        if (initialDimensions.find("auto") != std::string::npos) {
            std::string autoStr;
            dimensionsStream >> autoStr;
            ValidateMsg(dimensionsStream && autoStr == "auto", "Invalid packing dimensions format. "
                                                               "Expected: {auto|[dim x] [dim y] [dim z]}");
            return {0, 0, 0};
        } else {
            dimensionsStream >> dimensions[0] >> dimensions[1] >> dimensions[2];
            ValidateMsg(dimensionsStream, "Invalid packing dimensions format. "
                                          "Expected: {auto|[dim x] [dim y] [dim z]}");
            Validate(std::all_of(dimensions.begin(), dimensions.end(), [](double d) { return d > 0; }));
            return dimensions;
        }
    }

    OrthorhombicArrangingModel::Axis parse_axis(const std::string &axisStr) {
        OrthorhombicArrangingModel::Axis tiltAxis;
        if (axisStr == "x")
            tiltAxis = OrthorhombicArrangingModel::Axis::X;
        else if (axisStr == "y")
            tiltAxis = OrthorhombicArrangingModel::Axis::Y;
        else if (axisStr == "z")
            tiltAxis = OrthorhombicArrangingModel::Axis::Z;
        else
            throw ValidationException("Only x, y, z tilt axes are allowed");
        return tiltAxis;
    }

    auto parse_polarization(std::istringstream &arrangementStream)  {
        auto polarization = OrthorhombicArrangingModel::Polarization::IMPLICIT;
        auto axis = OrthorhombicArrangingModel::Axis::X;
        if (arrangementStream.str().find(" polar ") != std::string::npos)
            polarization = OrthorhombicArrangingModel::Polarization::FERRO;
        else if (arrangementStream.str().find(" antipolar ") != std::string::npos)
            polarization = OrthorhombicArrangingModel::Polarization::ANTIFERRO;
        else
            return std::make_pair(polarization, axis);

        std::string polarizationStr;
        std::string axisStr;
        arrangementStream >> polarizationStr >> axisStr;
        ValidateMsg(arrangementStream && (polarizationStr == "polar" || polarizationStr == "antipolar"),
                    ORTHORHOMBIC_USAGE);
        axis = parse_axis(axisStr);

        return std::make_pair(polarization, axis);
    }

    auto parse_tilt(std::istringstream &arrangementStream)  {
        auto clinicity = OrthorhombicArrangingModel::Clinicity::IMPLICIT;
        auto tiltAxis = OrthorhombicArrangingModel::Axis::X;
        double tiltAngle = 0;
        if (arrangementStream.str().find(" synclinic ") != std::string::npos)
            clinicity = OrthorhombicArrangingModel::Clinicity::SYNCLINIC;
        else if (arrangementStream.str().find(" anticlinic ") != std::string::npos)
            clinicity = OrthorhombicArrangingModel::Clinicity::ANTICLINIC;
        else
            return std::make_tuple(clinicity, tiltAxis, tiltAngle);

        std::string clinicityStr;
        std::string axisStr;
        arrangementStream >> clinicityStr >> axisStr >> tiltAngle;
        ValidateMsg(arrangementStream && (clinicityStr == "synclinic" || clinicityStr == "anticlinic"),
                    ORTHORHOMBIC_USAGE);
        tiltAxis = parse_axis(axisStr);

        return std::make_tuple(clinicity, tiltAxis, tiltAngle);
    }

    std::vector<Shape> parse_default(std::istringstream &arrangementStream, size_t numOfParticles,
                                     const std::array<double, 3> &boxDimensions,
                                     const OrthorhombicArrangingModel &model)
    {
        std::string defaultStr;
        arrangementStream >> defaultStr;
        ValidateMsg(arrangementStream && defaultStr == "default", ORTHORHOMBIC_USAGE);
        ValidateMsg((boxDimensions != std::array<double, 3>{0, 0, 0}),
                    "Default arrangement unsupported for automatic box size");

        return model.arrange(numOfParticles, boxDimensions);
    }

    std::array<double, 3> find_minimal_distances(std::size_t numOfParticles, const Interaction &interaction,
                                                 std::array<std::size_t, 3> &particlesInLine,
                                                 const std::string &axisOrderString,
                                                 const OrthorhombicArrangingModel &model)
    {
        double rangeRadius = interaction.getTotalRangeRadius();
        constexpr double EPSILON = 1e-12;

        std::array<double, 3> testPackingCellDim{};
        testPackingCellDim.fill(rangeRadius + EPSILON);
        std::array<double, 3> initialTestPackingDim{};
        std::transform(testPackingCellDim.begin(), testPackingCellDim.end(), particlesInLine.begin(),
                       initialTestPackingDim.begin(), [](double dim, std::size_t num) { return dim*num; });
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();
        auto testShapes = model.arrange(numOfParticles, particlesInLine, testPackingCellDim, initialTestPackingDim);
        Packing testPacking(initialTestPackingDim, testShapes, std::move(pbc), interaction);

        DistanceOptimizer::shrinkPacking(testPacking, interaction, axisOrderString);
        std::array<double, 3> minDistances{};
        std::array<double, 3> finalTestPackingDim = testPacking.getBox().getHeights();
        std::transform(finalTestPackingDim.begin(), finalTestPackingDim.end(), particlesInLine.begin(),
                       minDistances.begin(), [](double dim, std::size_t num) { return dim/num; });
        return minDistances;
    }

    std::vector<Shape> parse_spacing(std::istringstream &arrangementStream, std::size_t numOfParticles,
                                     std::array<double, 3> &boxDimensions, const Interaction &interaction,
                                     const OrthorhombicArrangingModel &model)
    {
        std::string spacingStr;
        double spacing;
        std::array<std::size_t, 3> particlesInLine{};
        std::string axisOrderString;
        arrangementStream >> spacingStr >> spacing >> axisOrderString;
        arrangementStream >> particlesInLine[0] >> particlesInLine[1] >> particlesInLine[2];
        ValidateMsg(arrangementStream && spacingStr == "spacing", ORTHORHOMBIC_USAGE);
        Validate(spacing > 0);
        Validate(std::accumulate(particlesInLine.begin(), particlesInLine.end(), 1., std::multiplies<>{})
                 >= numOfParticles);

        std::array<double, 3> distances = find_minimal_distances(numOfParticles, interaction, particlesInLine,
                                                                 axisOrderString, model);
        std::array<double, 3> cellDimensions{};
        std::transform(distances.begin(), distances.end(), cellDimensions.begin(),
                       [spacing](double distance) { return distance + spacing; });
        std::transform(cellDimensions.begin(), cellDimensions.end(), particlesInLine.begin(), boxDimensions.begin(),
                       std::multiplies<>{});
        return model.arrange(numOfParticles, particlesInLine, cellDimensions, boxDimensions);
    }

    std::vector<Shape> parse_explicit_sizes(std::istringstream &arrangementStream, std::size_t numOfParticles,
                                            std::array<double, 3> &boxDimensions,
                                            const OrthorhombicArrangingModel &model)
    {
        std::array<std::string, 3> firstThreeTokens{};
        arrangementStream >> firstThreeTokens[0] >> firstThreeTokens[1] >> firstThreeTokens[2];
        ValidateMsg(arrangementStream, ORTHORHOMBIC_USAGE);

        std::array<double, 3> cellDimensions{};
        std::array<std::size_t, 3> particlesInLine{};
        if (!ParseUtils::isAnythingLeft(arrangementStream)) {
            // Format: [num particles x] [... y] [... z]
            ValidateMsg((boxDimensions != std::array<double, 3>{0, 0, 0}),
                        "Implicit cell sizes are not available for automatic box dimensions");
            for (std::size_t i{}; i < 3; i++) {
                particlesInLine[i] = std::stoul(firstThreeTokens[i]);
                cellDimensions[i] = boxDimensions[i] / static_cast<double>(particlesInLine[i]);
            }
        } else {
            // Format: [cell size x] [... y] [... z] [num particles x] [... y] [... z]
            arrangementStream >> particlesInLine[0] >> particlesInLine[1] >> particlesInLine[2];
            ValidateMsg(arrangementStream, ORTHORHOMBIC_USAGE);
            for (std::size_t i{}; i < 3; i++)
                cellDimensions[i] = std::stod(firstThreeTokens[i]);
            Validate(std::all_of(cellDimensions.begin(), cellDimensions.end(), [](double d) { return d > 0; }));

            if (boxDimensions == std::array<double, 3>{0, 0, 0}) {
                std::transform(cellDimensions.begin(), cellDimensions.end(), particlesInLine.begin(),
                               boxDimensions.begin(), std::multiplies<>{});
            }
        }

        Validate(std::accumulate(particlesInLine.begin(), particlesInLine.end(), 1., std::multiplies<>{})
                 >= numOfParticles);

        return model.arrange(numOfParticles, particlesInLine, cellDimensions, boxDimensions);
    }

    std::vector<Shape> arrange_orthorhombic_shapes(std::size_t numOfParticles, std::array<double, 3> &boxDimensions,
                                                   const Interaction &interaction,
                                                   std::istringstream &arrangementStream)
    {
        auto [clinicity, tiltAxis, tiltAngle] = parse_tilt(arrangementStream);
        auto [polarization, polarAxis] = parse_polarization(arrangementStream);


        if (clinicity != OrthorhombicArrangingModel::Clinicity::IMPLICIT &&
            polarization == OrthorhombicArrangingModel::Polarization::IMPLICIT)
        {
            throw ValidationException("Polarization must be specified explicitly for non-implicit clinicity!");
        }

        OrthorhombicArrangingModel model(polarization, polarAxis, clinicity, tiltAxis, tiltAngle);
        std::vector<Shape> shapes;
        if (arrangementStream.str().find(" default") != std::string::npos)
            return parse_default(arrangementStream, numOfParticles, boxDimensions, model);
        else if (arrangementStream.str().find(" spacing ") != std::string::npos)
            return parse_spacing(arrangementStream, numOfParticles, boxDimensions, interaction, model);
        else
            return parse_explicit_sizes(arrangementStream, numOfParticles, boxDimensions, model);
    }
}


namespace legacy {
    std::unique_ptr<Packing> ArrangementFactory::arrangePacking(std::size_t numOfParticles,
                                                                const std::string &boxString,
                                                                const std::string &arrangementString,
                                                                std::unique_ptr<BoundaryConditions> bc,
                                                                const ShapeTraits &shapeTraits, std::size_t moveThreads,
                                                                std::size_t scalingThreads)
    {
        const auto &interaction = shapeTraits.getInteraction();

        std::istringstream arrangementStream(arrangementString);
        std::string type;
        arrangementStream >> type;
        ValidateMsg(arrangementStream, "Malformed arrangement. Usage: [type: orthorhombic, presimulated] "
                                       "(type dependent parameters)");
        if (type == "orthorhombic" || type == "lattice") {
            auto boxDimensions = parse_box_dimensions(boxString);
            auto shapes = arrange_orthorhombic_shapes(numOfParticles, boxDimensions, interaction, arrangementStream);
            return std::make_unique<Packing>(boxDimensions, std::move(shapes), std::move(bc), interaction, moveThreads,
                                             scalingThreads);
        } else if (type == "presimulated") {
            std::string filename;
            arrangementStream >> filename;
            ValidateMsg(arrangementStream, "Malformed presimulated arrangement. Usage: presimulated [RAMSNAP file]");

            std::ifstream packingFile(filename);
            ValidateOpenedDesc(packingFile, filename, "to load initial configuration");

            auto packing = std::make_unique<Packing>(std::move(bc), moveThreads, scalingThreads);
            packing->restore(packingFile, interaction);
            return packing;
        } else {
            const auto &supportedTypes = LatticeBuilder::getSupportedCellTypes();
            if (std::find(supportedTypes.begin(), supportedTypes.end(), type) != supportedTypes.end()) {
                return LatticeBuilder::buildPacking(numOfParticles, boxString, arrangementString, std::move(bc),
                                                    shapeTraits, moveThreads, scalingThreads);
            } else {
                throw ValidationException("Unknown arrangement type: " + type + ". Available: orthorhombic, "
                                          + "presimulated");
            }
        }
    }
}
