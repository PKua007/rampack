//
// Created by Piotr Kubala on 22/12/2020.
//

#include <numeric>
#include <algorithm>
#include <iterator>
#include <functional>
#include <sstream>

#include "PolysphereTraits.h"
#include "utils/Exceptions.h"
#include "XCObjShapePrinter.h"
#include "geometry/xenocollide/XCPrimitives.h"
#include "utils/Utils.h"


// PolysphereShape::SphereData #########################################################################################

PolysphereShape::SphereData::SphereData(const Vector<3> &position, double radius)
        : position{position}, radius{radius}
{
    Expects(radius > 0);
}

void PolysphereShape::SphereData::toWolfram(std::ostream &out, const Shape &shape) const {
    out << "Sphere[" << this->centreForShape(shape) << "," << this->radius << "]";
}

Vector<3> PolysphereShape::SphereData::centreForShape(const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation() * this->position;
}


// PolysphereShape #####################################################################################################

PolysphereShape::PolysphereShape(std::vector<SphereData> sphereData, OptionalAxis primaryAxis,
                                 OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin,
                                 std::optional<double> volume,
                                 const std::map<std::string, Vector<3>> &customNamedPoints)
        : sphereData{std::move(sphereData)}, primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis},
          geometricOrigin{geometricOrigin}, customNamedPoints{customNamedPoints}
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
}

Vector<3> PolysphereShape::getPrimaryAxis() const {
    if (!this->primaryAxis.has_value())
        throw std::runtime_error("PolysphereShape::getPrimaryAxis: primary axis not defined");
    return this->primaryAxis.value();
}

Vector<3> PolysphereShape::getSecondaryAxis() const {
    if (!this->secondaryAxis.has_value())
        throw std::runtime_error("PolysphereShape::getSecondaryAxis: secondary axis not defined");
    return this->secondaryAxis.value();
}

std::vector<Vector<3>> PolysphereShape::getInteractionCentres() const {
    std::vector<Vector<3>> centres;
    centres.reserve(this->sphereData.size());
    std::transform(this->sphereData.begin(), this->sphereData.end(), std::back_inserter(centres),
                   std::mem_fn(&SphereData::position));
    return centres;
}

double PolysphereShape::calculateVolume() const {
    ExpectsMsg(!this->spheresOverlap(),
               "PolysphereShape::calculateVolume: automatic volume not supported for overlapping spheres");

    auto volumeAccumulator = [](double volume_, const SphereData &data) {
        return volume_ + 4 * M_PI / 3 * data.radius * data.radius * data.radius;
    };
    return std::accumulate(this->sphereData.begin(), this->sphereData.end(), 0., volumeAccumulator);
}

void PolysphereShape::normalizeMassCentre() {
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

    for (auto &[name, point] : this->customNamedPoints)
        point -= massCentre;
}

Vector<3> PolysphereShape::calculateMassCentre() const {
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

bool PolysphereShape::spheresOverlap() const {
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

void PolysphereShape::addCustomNamedPoints(std::map<std::string, Vector<3>> namedPoints) {
    namedPoints.merge(std::move(this->customNamedPoints));
    this->customNamedPoints = std::move(namedPoints);
}

bool operator==(const PolysphereShape &lhs, const PolysphereShape &rhs) {
    return std::tie(
        lhs.sphereData, lhs.primaryAxis, lhs.secondaryAxis, lhs.geometricOrigin, lhs.volume, lhs.customNamedPoints
    ) == std::tie(
        rhs.sphereData, rhs.primaryAxis, rhs.secondaryAxis, rhs.geometricOrigin, rhs.volume, rhs.customNamedPoints
    );
}


// PolysphereTraits::WolframPrinter ####################################################################################

std::string PolysphereTraits::WolframPrinter::print(const Shape &shape) const {
    const auto &sphereData = this->traits.shapeFor(shape).getSphereData();

    std::ostringstream out;
    out << std::fixed;
    out << "{";
    for (std::size_t i{}; i < sphereData.size() - 1; i++) {
        const auto &data = sphereData[i];
        data.toWolfram(out, shape);
        out << ",";
    }
    sphereData.back().toWolfram(out, shape);
    out << "}";
    return out.str();
}


// PolysphereTraits::HardInteraction ###################################################################################

bool PolysphereTraits::HardInteraction::overlapBetween(const Vector<3> &pos1,
                                                       [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                       const std::byte *data1, std::size_t idx1,
                                                       const Vector<3> &pos2,
                                                       [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                       const std::byte *data2, std::size_t idx2,
                                                       const BoundaryConditions &bc) const
{
    const auto &sphereData1 = this->traits.shapeFor(data1).getSphereData();
    const auto &sphereData2 = this->traits.shapeFor(data2).getSphereData();

    double r = sphereData1[idx1].radius + sphereData2[idx2].radius;
    return bc.getDistance2(pos1, pos2) < r * r;
}

std::vector<Vector<3>> PolysphereTraits::HardInteraction::getInteractionCentres(const std::byte *data) const {
    const auto &shape = this->traits.shapeFor(data);
    return shape.getInteractionCentres();
}

double PolysphereTraits::HardInteraction::getRangeRadius(const std::byte *data) const {
    const auto &sphereData = this->traits.shapeFor(data).getSphereData();

    auto comparator = [](const SphereData &sd1, const SphereData &sd2) {
        return sd1.radius < sd2.radius;
    };
    return 2 * std::max_element(sphereData.begin(), sphereData.end(), comparator)->radius;
}

bool PolysphereTraits::HardInteraction::overlapWithWall(const Vector<3> &pos,
                                                        [[maybe_unused]] const Matrix<3, 3> &orientation,
                                                        const std::byte *data, std::size_t idx,
                                                        const Vector<3> &wallOrigin, const Vector<3> &wallVector) const
{
    const auto &sphereData = this->traits.shapeFor(data).getSphereData();

    double dotProduct = wallVector * (pos - wallOrigin);
    return dotProduct < sphereData[idx].radius;
}


// PolysphereTraits ####################################################################################################

void PolysphereTraits::registerSphereNamedPoint(std::size_t sphereIdx) {
    std::string pointName = "s" + std::to_string(sphereIdx);
    if (this->hasNamedPoint(pointName))
        return;

    this->registerDynamicNamedPoint(pointName, [this, sphereIdx, pointName](const ShapeData &data) -> Vector<3> {
        std::size_t shapeIdx = data.as<Data>().shapeIdx;
        const auto &sphereData = this->getShape(shapeIdx).getSphereData();
        if (sphereIdx >= sphereData.size())
            this->throwUnavailableNamedPoint(shapeIdx, pointName);

        return sphereData[sphereIdx].position;
    });
}

PolysphereTraits::PolysphereTraits()
        : interaction{std::make_shared<HardInteraction>(*this)}, centralInteraction{nullptr},
          wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{ }

PolysphereTraits::PolysphereTraits(const PolysphereShape &polysphereShape) : PolysphereTraits() {
    this->addShape("A", polysphereShape);
    this->setDefaultShapeData({{"type", "A"}});
}

PolysphereTraits::PolysphereTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
        : interaction{centralInteraction}, centralInteraction{centralInteraction},
          wolframPrinter{std::make_shared<WolframPrinter>(*this)}
{
    this->centralInteraction->installCentresProvider([this](const std::byte *data) {
        return this->shapeFor(data).getInteractionCentres();
    });
}

PolysphereTraits::PolysphereTraits(const PolysphereShape &polysphereShape,
                                   const std::shared_ptr<CentralInteraction> &centralInteraction)
        : PolysphereTraits(centralInteraction)
{
    this->addShape("A", polysphereShape);
    this->setDefaultShapeData({{"type", "A"}});
}

PolysphereTraits::~PolysphereTraits() {
    if (this->centralInteraction)
        this->centralInteraction->detach();
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
    // TODO: obj printer
    /*else if (format == "obj")
        return this->createObjPrinter(meshSubdivisions);*/
    else
        throw NoSuchShapePrinterException("PolysphereTraits: unknown printer format: " + format);
}

ShapeData PolysphereTraits::addShape(const std::string &shapeName, const PolysphereShape &shape) {
    for (std::size_t sphereIdx{}; sphereIdx < shape.getSphereData().size(); sphereIdx++)
        this->registerSphereNamedPoint(sphereIdx);

    return GenericShapeRegistry::addShape(shapeName, shape);
}

/*std::shared_ptr<ShapePrinter> PolysphereTraits::createObjPrinter(std::size_t subdivisions) const {
    const auto &sphereData = this->getSphereData();

    std::vector<XCSphere> xcSpheres;
    std::vector<const AbstractXCGeometry *> geometries;
    xcSpheres.reserve(sphereData.size());
    geometries.reserve(sphereData.size());
    for (const auto &sphereDataEntry : sphereData) {
        xcSpheres.emplace_back(sphereDataEntry.radius);
        geometries.push_back(&xcSpheres.back());
    }

    auto interactionCentres = this->interaction->getInteractionCentres(nullptr);

    return std::make_shared<XCObjShapePrinter>(geometries, interactionCentres, subdivisions);
}*/
