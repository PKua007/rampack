//
// Created by Piotr Kubala on 22/12/2020.
//

#include <numeric>
#include <algorithm>
#include <iterator>

#include "PolysphereTraits.h"
#include "utils/Exceptions.h"
#include "XCObjShapePrinter.h"
#include "geometry/xenocollide/XCPrimitives.h"


std::string PolysphereTraits::WolframPrinter::print(const Shape &shape) const {
    std::ostringstream out;
    out << std::fixed;
    out << "{";
    const auto &sphereData = this->traits.getSphereData();
    for (std::size_t i{}; i < sphereData.size() - 1; i++) {
        const auto &data = sphereData[i];
        data.toWolfram(out, shape);
        out << ",";
    }
    sphereData.back().toWolfram(out, shape);
    out << "}";
    return out.str();
}

PolysphereTraits::PolysphereTraits(PolysphereTraits::PolysphereGeometry geometry,
                                   std::shared_ptr<CentralInteractionBase> centralInteraction,
                                   bool allowUniformPairDataBroadcast)
        : geometry{std::move(geometry)}, wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{
    centralInteraction->bindCentreLayout(this->geometry.getInteractionCentreLayout(), allowUniformPairDataBroadcast);
    Expects(this->geometry.getInteractionCentreLayout() == centralInteraction->getInteractionCentreLayout());
    this->interaction = std::move(centralInteraction);
}

PolysphereTraits::PolysphereTraits(PolysphereTraits::PolysphereGeometry geometry)
    : geometry{std::move(geometry)}, wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{
    this->interaction = std::make_shared<HardInteraction>(this->geometry);
}

std::shared_ptr<const ShapePrinter>
PolysphereTraits::getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const {
    std::size_t meshSubdivisions = DEFAULT_MESH_SUBDIVISIONS;
    if (params.find("mesh_divisions") != params.end()) {
        meshSubdivisions = std::stoul(params.at("mesh_divisions"));
        Expects(meshSubdivisions >= 1);
    }

    if (format == "wolfram")
        return this->wolframPrinter;
    else if (format == "obj")
        return this->createObjPrinter(meshSubdivisions);
    else
        throw NoSuchShapePrinterException("PolysphereTraits: unknown printer format: " + format);
}

std::shared_ptr<ShapePrinter> PolysphereTraits::createObjPrinter(std::size_t subdivisions) const {
    const auto &sphereData = this->getSphereData();

    std::vector<XCSphere> xcSpheres;
    std::vector<const AbstractXCGeometry *> geometries;
    xcSpheres.reserve(sphereData.size());
    geometries.reserve(sphereData.size());
    for (const auto &sphereDataEntry : sphereData) {
        xcSpheres.emplace_back(sphereDataEntry.radius);
        geometries.push_back(&xcSpheres.back());
    }

    auto interactionCentres = this->interaction->getInteractionCentres();

    return std::make_shared<XCObjShapePrinter>(geometries, interactionCentres, subdivisions);
}

PolysphereTraits::SphereData::SphereData(const Vector<3> &position, double radius)
        : position{position}, radius{radius}
{
    Expects(radius > 0);
}

PolysphereTraits::InteractionCentreTypeMetadata::InteractionCentreTypeMetadata(double radius)
        : radius{radius}
{
    Expects(radius > 0);
}

PolysphereTraits::InteractionCentreLayoutWithMetadata::InteractionCentreLayoutWithMetadata(
        InteractionCentreLayout interactionCentreLayout,
        std::vector<InteractionCentreTypeMetadata> centreTypeMetadata)
        : interactionCentreLayout{std::move(interactionCentreLayout)}, centreTypeMetadata{std::move(centreTypeMetadata)}
{
    Expects(this->interactionCentreLayout.numCentreTypes() == this->centreTypeMetadata.size());
}

void PolysphereTraits::SphereData::toWolfram(std::ostream &out, const Shape &shape) const {
    out << "Sphere[" << this->centreForShape(shape) << "," << this->radius << "]";
}

Vector<3> PolysphereTraits::SphereData::centreForShape(const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation() * this->position;
}

bool PolysphereTraits::HardInteraction::overlapBetween(const Vector<3> &pos1,
                                                       [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                       std::size_t idx1,
                                                       const Vector<3> &pos2,
                                                       [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                       std::size_t idx2, const BoundaryConditions &bc) const
{
    double r = this->radii[idx1] + this->radii[idx2];
    return bc.getDistance2(pos1, pos2) < r * r;
}

std::vector<Vector<3>> PolysphereTraits::HardInteraction::getInteractionCentres() const {
    return this->interactionCentres;
}

double PolysphereTraits::HardInteraction::getRangeRadius() const {
    return 2 * *std::max_element(this->radii.begin(), this->radii.end());
}

PolysphereTraits::HardInteraction::HardInteraction(const PolysphereGeometry &geometry)
{
    const auto &sphereData = geometry.getSphereData();
    this->interactionCentres.reserve(sphereData.size());
    this->radii.reserve(sphereData.size());
    for (const auto &sphere : sphereData) {
        this->interactionCentres.push_back(sphere.position);
        this->radii.push_back(sphere.radius);
    }
}

bool PolysphereTraits::HardInteraction::overlapWithWall(const Vector<3> &pos,
                                                        [[maybe_unused]] const Matrix<3, 3> &orientation,
                                                        std::size_t idx, const Vector<3> &wallOrigin,
                                                        const Vector<3> &wallVector) const
{
    double dotProduct = wallVector * (pos - wallOrigin);
    return dotProduct < this->radii[idx];
}

double PolysphereTraits::PolysphereGeometry::calculateVolume() const {
    ExpectsMsg(!this->spheresOverlap(), "PolysphereTraits::PolysphereGeometry::calculateVolume: automatic volume "
                                        "not supported for overlapping spheres");

    auto volumeAccumulator = [](double volume_, const SphereData &data) {
        return volume_ + 4 * M_PI / 3 * data.radius * data.radius * data.radius;
    };
    return std::accumulate(this->sphereData.begin(), this->sphereData.end(), 0., volumeAccumulator);
}

PolysphereTraits::InteractionCentreLayoutWithMetadata
PolysphereTraits::PolysphereGeometry::sphereDataToInteractionCentreLayoutWithMetadata(std::vector<SphereData> sphereData)
{
    Expects(!sphereData.empty());
    std::vector<Vector<3>> centres;
    std::vector<std::size_t> centreIdxTypeMap;
    std::vector<InteractionCentreTypeMetadata> centreTypeMetadata;
    centres.reserve(sphereData.size());
    centreIdxTypeMap.reserve(sphereData.size());
    centreTypeMetadata.reserve(sphereData.size());
    for (std::size_t i{}; i < sphereData.size(); i++) {
        centres.push_back(sphereData[i].position);
        centreIdxTypeMap.push_back(i);
        centreTypeMetadata.emplace_back(sphereData[i].radius);
    }
    return {InteractionCentreLayout{std::move(centres), std::move(centreIdxTypeMap)}, std::move(centreTypeMetadata)};
}

void PolysphereTraits::PolysphereGeometry::normalizeMassCentre() {
    Vector<3> massCentre = this->calculateMassCentre();

    auto massCentreShifter = [massCentre](const SphereData &data) {
        return SphereData(data.position - massCentre, data.radius);
    };

    std::vector<SphereData> newSphereData;
    newSphereData.reserve(this->sphereData.size());
    std::transform(this->sphereData.begin(), this->sphereData.end(), std::back_inserter(newSphereData),
                   massCentreShifter);

    this->sphereData = std::move(newSphereData);
    std::vector<Vector<3>> shiftedCentres;
    shiftedCentres.reserve(this->interactionCentreLayout.numCentres());
    std::transform(this->interactionCentreLayout.getCentres().begin(), this->interactionCentreLayout.getCentres().end(),
                   std::back_inserter(shiftedCentres),
                   [massCentre](const Vector<3> &centre) { return centre - massCentre; });
    this->interactionCentreLayout = {std::move(shiftedCentres), this->interactionCentreLayout.getCentreIdxTypeMap()};
    this->geometricOrigin -= massCentre;
    this->moveNamedPoints(-massCentre);
}

PolysphereTraits::PolysphereGeometry::PolysphereGeometry(std::vector<SphereData> sphereData, OptionalAxis primaryAxis,
                                                         OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin,
                                                         std::optional<double> volume,
                                                         const ShapeGeometry::NamedPoints &customNamedPoints)
        : PolysphereGeometry(PolysphereGeometry::sphereDataToInteractionCentreLayoutWithMetadata(std::move(sphereData)), primaryAxis,
                             secondaryAxis, geometricOrigin, volume, customNamedPoints)
{ }

PolysphereTraits::PolysphereGeometry::PolysphereGeometry(
        InteractionCentreLayoutWithMetadata interactionCentreLayoutWithMetadata, OptionalAxis primaryAxis,
        OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin, std::optional<double> volume,
        const ShapeGeometry::NamedPoints &customNamedPoints)
        : interactionCentreLayout{interactionCentreLayoutWithMetadata.getInteractionCentreLayout()},
          displayRadiiByType(interactionCentreLayoutWithMetadata.getCentreTypeMetadata().size()),
          primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis}, geometricOrigin{geometricOrigin}
{
    Expects(this->interactionCentreLayout.numCentres() > 0);
    if (!this->primaryAxis.has_value())
        Expects(!this->secondaryAxis.has_value());
    if (this->primaryAxis.has_value())
        this->primaryAxis = this->primaryAxis->normalized();
    if (this->secondaryAxis.has_value())
        this->secondaryAxis = this->secondaryAxis->normalized();

    const auto &centres = this->interactionCentreLayout.getCentres();
    const auto &centreIdxTypeMap = this->interactionCentreLayout.getCentreIdxTypeMap();
    const auto &centreTypeMetadata = interactionCentreLayoutWithMetadata.getCentreTypeMetadata();
    this->sphereData.reserve(centres.size());
    for (std::size_t i{}; i < centres.size(); i++) {
        double radius = centreTypeMetadata[centreIdxTypeMap[i]].radius;
        this->sphereData.emplace_back(centres[i], radius);
        this->registerNamedPoint("s" + std::to_string(i), centres[i]);
    }

    std::transform(centreTypeMetadata.begin(), centreTypeMetadata.end(), this->displayRadiiByType.begin(),
                   [](const InteractionCentreTypeMetadata &metadata) { return metadata.radius; });

    if (volume.has_value())
        this->volume = *volume;
    else
        this->volume = this->calculateVolume();

    this->registerNamedPoints(customNamedPoints);
}

Vector<3> PolysphereTraits::PolysphereGeometry::calculateMassCentre() const {
    auto massCentreAccumulator = [](const Vector<3> &sum, const SphereData &data) {
        double r = data.radius;
        return sum + (r*r*r) * data.position;
    };

    auto weightAccumulator = [](double sum, const SphereData &data) {
        double r = data.radius;
        return sum + r*r*r;
    };

    Vector<3> massCentre = std::accumulate(this->sphereData.begin(), this->sphereData.end(), Vector<3>{},
                                           massCentreAccumulator);
    double weightSum = std::accumulate(this->sphereData.begin(), this->sphereData.end(), 0., weightAccumulator);
    massCentre /= weightSum;
    return massCentre;
}

bool PolysphereTraits::PolysphereGeometry::spheresOverlap() const {
    for (std::size_t i{}; i < this->sphereData.size(); i++) {
        for (std::size_t j = i + 1; j < this->sphereData.size(); j++) {
            const auto &data1 = this->sphereData[i];
            const auto &data2 = this->sphereData[j];

            constexpr double EPSILON = 1e-12;
            double distance2 = (data1.position - data2.position).norm2();
            double radii2 = std::pow(data1.radius + data2.radius, 2);
            if (distance2 * (EPSILON + 1) < radii2)
                return true;
        }
    }

    return false;
}
