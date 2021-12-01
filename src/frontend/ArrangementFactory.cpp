//
// Created by pkua on 01.12.2021.
//

#include <numeric>
#include <fstream>

#include "ArrangementFactory.h"
#include "core/arranging_models/OrthorombicArrangingModel.h"
#include "core/MinimalDistanceOptimizer.h"
#include "utils/Assertions.h"


#define ORTHOROMBIC_USAGE "Malformed latice arrangement. Usage: \n" \
                          "orthorombic (({synclinic|anticlinic} {x|y|z} [tile angle]) {polar|antipolar} {x|y|z}) ...\n" \
                          "  ... {default|spacing [space size] [num. of particles in line x] [... y] [... z]|" \
                          "[cell size x] [... y] [... z] [num. of particles in line x] [... y] [... z]}"

namespace {
    OrthorombicArrangingModel::Axis parse_axis(const std::string &axisStr) {
        OrthorombicArrangingModel::Axis tiltAxis;
        if (axisStr == "x")
            tiltAxis = OrthorombicArrangingModel::Axis::X;
        else if (axisStr == "y")
            tiltAxis = OrthorombicArrangingModel::Axis::Y;
        else if (axisStr == "z")
            tiltAxis = OrthorombicArrangingModel::Axis::Z;
        else
            throw ValidationException("Only x, y, z tilt axes are allowed");
        return tiltAxis;
    }

    auto parse_polarization(std::istringstream &arrangementStream)  {
        auto polarization = OrthorombicArrangingModel::Polarization::IMPLICIT;
        auto axis = OrthorombicArrangingModel::Axis::X;
        if (arrangementStream.str().find(" polar ") != std::string::npos)
            polarization = OrthorombicArrangingModel::Polarization::FERRO;
        else if (arrangementStream.str().find(" antipolar ") != std::string::npos)
            polarization = OrthorombicArrangingModel::Polarization::ANTIFERRO;
        else
            return std::make_pair(polarization, axis);

        std::string polarizationStr;
        std::string axisStr;
        arrangementStream >> polarizationStr >> axisStr;
        ValidateMsg(arrangementStream && (polarizationStr == "polar" || polarizationStr == "antipolar"),
                    ORTHOROMBIC_USAGE);
        axis = parse_axis(axisStr);

        return std::make_pair(polarization, axis);
    }

    auto parse_tilt(std::istringstream &arrangementStream)  {
        auto clinicity = OrthorombicArrangingModel::Clinicity::IMPLICIT;
        auto tiltAxis = OrthorombicArrangingModel::Axis::X;
        double tiltAngle = 0;
        if (arrangementStream.str().find(" synclinic ") != std::string::npos)
            clinicity = OrthorombicArrangingModel::Clinicity::SYNCLINIC;
        else if (arrangementStream.str().find(" anticlinic ") != std::string::npos)
            clinicity = OrthorombicArrangingModel::Clinicity::ANTICLINIC;
        else
            return std::make_tuple(clinicity, tiltAxis, tiltAngle);

        std::string clinicityStr;
        std::string axisStr;
        arrangementStream >> clinicityStr >> axisStr >> tiltAngle;
        ValidateMsg(arrangementStream && (clinicityStr == "synclinic" || clinicityStr == "anticlinic"),
                    ORTHOROMBIC_USAGE);
        tiltAxis = parse_axis(axisStr);

        return std::make_tuple(clinicity, tiltAxis, tiltAngle);
    }

    std::vector<Shape> parse_default(std::istringstream &arrangementStream, size_t numOfParticles,
                                     const std::array<double, 3> &boxDimensions,
                                     const OrthorombicArrangingModel &model)
    {
        std::string defaultStr;
        arrangementStream >> defaultStr;
        ValidateMsg(arrangementStream && defaultStr == "default", ORTHOROMBIC_USAGE);
        ValidateMsg((boxDimensions != std::array<double, 3>{0, 0, 0}),
                    "Default arrangement unsupported for automatic box size");

        return model.arrange(numOfParticles, boxDimensions);
    }

    std::array<double, 3> compute_distancec(OrthorombicArrangingModel::Polarization polarization,
                                            OrthorombicArrangingModel::Axis polarAxis, const Interaction &interaction)
    {
        Shape s1, s2;
        auto distances = MinimalDistanceOptimizer::forAxes(s1, s2, interaction);
        if (polarization == OrthorombicArrangingModel::Polarization::ANTIFERRO) {
            std::size_t axisNum = OrthorombicArrangingModel::getAxisNumber(polarAxis);

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
        return distances;
    }

    std::vector<Shape> parse_spacing(std::istringstream &arrangementStream, size_t numOfParticles,
                                     std::array<double, 3> &boxDimensions,
                                     OrthorombicArrangingModel::Polarization polarization,
                                     OrthorombicArrangingModel::Axis polarAxis, const Interaction &interaction,
                                     const OrthorombicArrangingModel &model)
    {
        std::string spacingStr;
        double spacing;
        std::array<std::size_t, 3> particlesInLine{};
        arrangementStream >> spacingStr >> spacing;
        arrangementStream >> particlesInLine[0] >> particlesInLine[1] >> particlesInLine[2];
        ValidateMsg(arrangementStream && spacingStr == "spacing", ORTHOROMBIC_USAGE);
        Validate(spacing > 0);
        Validate(std::accumulate(particlesInLine.begin(), particlesInLine.end(), 1., std::multiplies<>{})
                 >= numOfParticles);

        std::array<double, 3> distances = compute_distancec(polarization, polarAxis, interaction);
        std::array<double, 3> cellDimensions{};
        std::transform(distances.begin(), distances.end(), cellDimensions.begin(),
                       [spacing](double distance) { return distance + spacing; });
        std::transform(cellDimensions.begin(), cellDimensions.end(), particlesInLine.begin(), boxDimensions.begin(),
                       std::multiplies<>{});
        return model.arrange(numOfParticles, particlesInLine, cellDimensions, boxDimensions);
    }

    std::vector<Shape> parse_explicit_sizes(std::istringstream &arrangementStream, std::size_t numOfParticles,
                                            std::array<double, 3> &boxDimensions,
                                            const OrthorombicArrangingModel &model)
    {
        std::array<double, 3> cellDimensions{};
        std::array<std::size_t, 3> particlesInLine{};
        arrangementStream >> cellDimensions[0] >> cellDimensions[1] >> cellDimensions[2];
        arrangementStream >> particlesInLine[0] >> particlesInLine[1] >> particlesInLine[2];
        ValidateMsg(arrangementStream, ORTHOROMBIC_USAGE);
        Validate(std::all_of(cellDimensions.begin(), cellDimensions.end(), [](double d) { return d; }));
        Validate(std::accumulate(particlesInLine.begin(), particlesInLine.end(), 1., std::multiplies<>{})
                 >= numOfParticles);

        if (boxDimensions == std::array<double, 3>{0, 0, 0}) {
            std::transform(cellDimensions.begin(), cellDimensions.end(), particlesInLine.begin(),
                           boxDimensions.begin(), std::multiplies<>{});
        }

        return model.arrange(numOfParticles, particlesInLine, cellDimensions, boxDimensions);
    }

    std::vector<Shape> arrange_orthorombic_shapes(std::size_t numOfParticles,
                                                  std::array<double, 3> &boxDimensions, const Interaction &interaction,
                                                  std::istringstream &arrangementStream)
    {
        auto [clinicity, tiltAxis, tiltAngle] = parse_tilt(arrangementStream);
        auto [polarization, polarAxis] = parse_polarization(arrangementStream);


        if (clinicity != OrthorombicArrangingModel::Clinicity::IMPLICIT &&
            polarization == OrthorombicArrangingModel::Polarization::IMPLICIT)
        {
            throw ValidationException("Polarization must be specified explicitly for non-implicit clinicity!");
        }

        OrthorombicArrangingModel model(polarization, polarAxis, clinicity, tiltAxis, tiltAngle);
        if (arrangementStream.str().find(" default ") != std::string::npos) {
            return parse_default(arrangementStream, numOfParticles, boxDimensions, model);
        } else if (arrangementStream.str().find(" spacing ") != std::string::npos) {
            return parse_spacing(arrangementStream, numOfParticles, boxDimensions, polarization, polarAxis, interaction,
                                 model);

        } else {
            return parse_explicit_sizes(arrangementStream, numOfParticles, boxDimensions, model);
        }
    }
}

std::unique_ptr<Packing> ArrangementFactory::arrangePacking(std::size_t numOfParticles,
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
        auto shapes = arrange_orthorombic_shapes(numOfParticles, boxDimensions, interaction,arrangementStream);
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
