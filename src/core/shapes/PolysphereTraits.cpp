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

std::string PolysphereTraits::toWolfram(const Shape &shape, double scale) const {
    std::ostringstream out;
    out << std::fixed;
    out << "{";
    for (std::size_t i{}; i < this->sphereData.size() - 1; i++) {
        const auto &data = this->sphereData[i];
        data.toWolfram(out, shape, scale);
        out << ",";
    }
    this->sphereData.back().toWolfram(out, shape, scale);
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

void PolysphereTraits::SphereData::toWolfram(std::ostream &out, const Shape &shape, double scale) const {
    out << "Sphere[" << this->centreForShape(shape, scale) << "," << this->radius << "]";
}

Vector<3> PolysphereTraits::SphereData::centreForShape(const Shape &shape, double scale) const {
    return shape.getPosition() * scale + shape.getOrientation() * this->position;
}

bool PolysphereTraits::HardInteraction::overlapBetween(const Shape &shape1, const Shape &shape2, double scale,
                                                       const BoundaryConditions &bc) const
{
    for (const auto &data1 : this->sphereData) {
        for (const auto &data2 : this->sphereData) {
            if (bc.getDistance2(data1.centreForShape(shape1, scale) / scale,
                                data2.centreForShape(shape2, scale) / scale) * scale * scale
                < (data1.radius + data2.radius) * (data1.radius + data2.radius))
            {
               return true;
            }
        }
    }

    return false;
}
