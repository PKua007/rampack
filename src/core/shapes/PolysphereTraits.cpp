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
                                   std::shared_ptr<CentralInteraction> centralInteraction)
        : geometry{std::move(geometry)}, wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{
    const auto &sphereData = this->getSphereData();
    std::vector<Vector<3>> centres;
    centres.reserve(sphereData.size());
    std::transform(sphereData.begin(), sphereData.end(), std::back_inserter(centres),
                   [](const SphereData &data) { return data.position; });
    centralInteraction->installOnCentres(centres);
    this->interaction = std::move(centralInteraction);
}

PolysphereTraits::PolysphereTraits(PolysphereTraits::PolysphereGeometry geometry)
    : geometry{std::move(geometry)}, wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{
    this->interaction = std::make_shared<HardInteraction>(this->getSphereData());
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

void PolysphereTraits::SphereData::toWolfram(std::ostream &out, const Shape &shape) const {
    out << "Sphere[" << this->centreForShape(shape) << "," << this->radius << "]";
}

Vector<3> PolysphereTraits::SphereData::centreForShape(const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation() * this->position;
}

bool PolysphereTraits::HardInteraction::overlapBetween(const Vector<3> &pos1,
                                                       [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                       [[maybe_unused]] const std::byte *data1,
                                                       std::size_t idx1,
                                                       const Vector<3> &pos2,
                                                       [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                       [[maybe_unused]] const std::byte *data2,
                                                       std::size_t idx2, const BoundaryConditions &bc) const
{
    double r = this->sphereData[idx1].radius + this->sphereData[idx2].radius;
    return bc.getDistance2(pos1, pos2) < r * r;
}

std::vector<Vector<3>> PolysphereTraits::HardInteraction::getInteractionCentres() const {
    std::vector<Vector<3>> centres;
    centres.reserve(this->sphereData.size());
    for (const auto &data : this->sphereData)
        centres.push_back(data.position);
    return centres;
}

double PolysphereTraits::HardInteraction::getRangeRadius() const {
    auto comparator = [](const SphereData &sd1, const SphereData &sd2) {
        return sd1.radius < sd2.radius;
    };
    return 2 * std::max_element(this->sphereData.begin(), this->sphereData.end(), comparator)->radius;
}

PolysphereTraits::HardInteraction::HardInteraction(std::vector<SphereData> sphereData)
        : sphereData{std::move(sphereData)}
{
    Expects(!this->sphereData.empty());
}

bool PolysphereTraits::HardInteraction::overlapWithWall(const Vector<3> &pos,
                                                        [[maybe_unused]] const Matrix<3, 3> &orientation,
                                                        [[maybe_unused]] const std::byte *data,
                                                        std::size_t idx, const Vector<3> &wallOrigin,
                                                        const Vector<3> &wallVector) const
{
    double dotProduct = wallVector * (pos - wallOrigin);
    return dotProduct < this->sphereData[idx].radius;
}

double PolysphereTraits::PolysphereGeometry::calculateVolume() const {
    ExpectsMsg(!this->spheresOverlap(), "PolysphereTraits::PolysphereGeometry::calculateVolume: automatic volume "
                                        "not supported for overlapping spheres");

    auto volumeAccumulator = [](double volume_, const SphereData &data) {
        return volume_ + 4 * M_PI / 3 * data.radius * data.radius * data.radius;
    };
    return std::accumulate(this->sphereData.begin(), this->sphereData.end(), 0., volumeAccumulator);
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
    this->geometricOrigin -= massCentre;
    this->moveNamedPoints(-massCentre);
}

PolysphereTraits::PolysphereGeometry::PolysphereGeometry(std::vector<SphereData> sphereData, OptionalAxis primaryAxis,
                                                         OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin,
                                                         std::optional<double> volume,
                                                         const ShapeGeometry::NamedPoints &customNamedPoints)
        : sphereData{std::move(sphereData)}, primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis},
          geometricOrigin{geometricOrigin}
{
    Expects(!this->sphereData.empty());
    if (!this->primaryAxis.has_value())
        Expects(!this->secondaryAxis.has_value());
    if (this->primaryAxis.has_value())
        this->primaryAxis = this->primaryAxis->normalized();
    if (this->secondaryAxis.has_value())
        this->secondaryAxis = this->secondaryAxis->normalized();

    if (volume.has_value())
        this->volume = *volume;
    else
        this->volume = this->calculateVolume();

    for (std::size_t i{}; i < this->sphereData.size(); i++) {
        const auto &ssData = this->sphereData[i];
        this->registerNamedPoint("s" + std::to_string(i), ssData.position);
    }

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
