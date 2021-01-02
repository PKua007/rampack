//
// Created by Piotr Kubala on 22/12/2020.
//

#include <numeric>
#include <algorithm>
#include <iterator>

#include "PolysphereTraits.h"
#include "utils/Assertions.h"

double PolysphereTraits::getVolume() const {
    auto volumeAccumulator = [](double volume, const SphereData &data) {
        return volume + 4*M_PI/3 * data.radius * data.radius * data.radius;
    };
    return std::accumulate(this->sphereData.begin(), this->sphereData.end(), 0., volumeAccumulator);
}

std::string PolysphereTraits::toWolfram(const Shape &shape) const {
    std::ostringstream out;
    out << std::fixed;
    out << "{";
    for (std::size_t i{}; i < this->sphereData.size() - 1; i++) {
        const auto &data = this->sphereData[i];
        data.toWolfram(out, shape);
        out << ",";
    }
    this->sphereData.back().toWolfram(out, shape);
    out << "}";
    return out.str();
}

PolysphereTraits::PolysphereTraits(std::vector<SphereData> sphereData,
                                   std::unique_ptr<CentralInteraction> centralInteraction)
        : sphereData{std::move(sphereData)}
{
    Expects(!this->sphereData.empty());
    std::vector<Vector<3>> centres;
    centres.reserve(this->sphereData.size());
    std::transform(this->sphereData.begin(), this->sphereData.end(), std::back_inserter(centres),
                   [](const SphereData &data) { return data.position; });
    centralInteraction->installOnCentres(centres);
    this->interaction = std::move(centralInteraction);
}

PolysphereTraits::PolysphereTraits(const std::vector<SphereData> &sphereData)
        : sphereData{sphereData}, interaction{std::make_unique<HardInteraction>(sphereData)}
{
    Expects(!sphereData.empty());
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
