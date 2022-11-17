//
// Created by Piotr Kubala on 22/12/2020.
//

#include <numeric>
#include <algorithm>
#include <iterator>

#include "PolysphereTraits.h"
#include "utils/Assertions.h"


std::string PolysphereTraits::toWolfram(const Shape &shape) const {
    std::ostringstream out;
    out << std::fixed;
    out << "{";
    const auto &sphereData = this->getSphereData();
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
                                   std::unique_ptr<CentralInteraction> centralInteraction)
        : geometry{std::move(geometry)}
{
    const auto &sphereData = this->getSphereData();
    std::vector<Vector<3>> centres;
    centres.reserve(sphereData.size());
    std::transform(sphereData.begin(), sphereData.end(), std::back_inserter(centres),
                   [](const SphereData &data) { return data.position; });
    centralInteraction->installOnCentres(centres);
    this->interaction = std::move(centralInteraction);
}

PolysphereTraits::PolysphereTraits(PolysphereTraits::PolysphereGeometry geometry) : geometry{std::move(geometry)} {
    this->interaction = std::make_unique<HardInteraction>(this->getSphereData());
}

const ShapePrinter &PolysphereTraits::getPrinter(const std::string &format) const {
    ExpectsMsg(format == "wolfram", "PolysphereTraits: unknown printer format: " + format);
    return *this;
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
                                                       std::size_t idx1,
                                                       const Vector<3> &pos2,
                                                       [[maybe_unused]] const Matrix<3, 3> &orientation2,
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
                                                        std::size_t idx, const Vector<3> &wallOrigin,
                                                        const Vector<3> &wallVector) const
{
    double dotProduct = wallVector * (pos - wallOrigin);
    return dotProduct < this->sphereData[idx].radius;
}

double PolysphereTraits::PolysphereGeometry::getVolume() const {
    auto volumeAccumulator = [](double volume, const SphereData &data) {
        return volume + 4*M_PI/3 * data.radius * data.radius * data.radius;
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

PolysphereTraits::PolysphereGeometry::PolysphereGeometry(std::vector<SphereData> sphereData,
                                                         const Vector<3> &primaryAxis, const Vector<3> &secondaryAxis,
                                                         const Vector<3> &geometricOrigin,
                                                         const ShapeGeometry::NamedPoints &customNamedPoints)
        : sphereData{std::move(sphereData)}, primaryAxis{primaryAxis.normalized()},
          secondaryAxis{secondaryAxis.normalized()}, geometricOrigin{geometricOrigin}
{
    Expects(!this->sphereData.empty());

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
