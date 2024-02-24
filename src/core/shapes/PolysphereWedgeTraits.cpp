//
// Created by pkua on 09.03.2022.
//

#include "PolysphereWedgeTraits.h"
#include "utils/Exceptions.h"
#include "geometry/VolumeCalculator.h"


namespace legacy {
    PolysphereTraits::PolysphereShape
    PolysphereWedgeTraits::generateShape(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                                         double spherePenetration)
    {
        Expects(sphereNum >= 2);
        Expects(smallSphereRadius > 0);
        Expects(largeSphereRadius > 0);
        ExpectsMsg(spherePenetration == 0,
                   "sphere penetration is disabled for version 0.1 because of incorrect volume calculation");

        std::vector<SphereData> data;

        double centrePos{};
        double radiusDelta = (largeSphereRadius - smallSphereRadius)/static_cast<double>(sphereNum - 1);
        double currentRadius = smallSphereRadius;
        for (std::size_t i{}; i < sphereNum; i++) {
            data.emplace_back(Vector<3>{centrePos, 0, 0}, currentRadius);
            centrePos += 2*currentRadius + radiusDelta - spherePenetration;
            currentRadius += radiusDelta;
        }

        PolysphereShape shape(std::move(data), {1, 0, 0}, {0, 1, 0}, {0, 0, 0});
        shape.normalizeMassCentre();
        double end1 = shape.getSphereData().front().position[0];
        double end2 = shape.getSphereData().back().position[0];
        shape.setGeometricOrigin({(end2 + largeSphereRadius + end1 - smallSphereRadius) / 2, 0, 0});
        shape.addCustomNamedPoints({{"cm", {0, 0, 0}},
                                    {"ss", shape.getSphereData().front().position},
                                    {"sl", shape.getSphereData().back().position}});
        return shape;
    }
}


PolysphereTraits::PolysphereShape
PolysphereWedgeTraits::generateShape(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                                     double spherePenetration)
{
    Expects(sphereNum >= 2);
    Expects(topSphereRadius > 0);
    Expects(bottomSphereRadius > 0);
    Expects(spherePenetration < 2*std::min(topSphereRadius, bottomSphereRadius));

    std::vector<SphereData> data;

    double centrePos{};
    double radiusDelta = (topSphereRadius - bottomSphereRadius) / static_cast<double>(sphereNum - 1);
    double currentRadius = bottomSphereRadius;
    for (std::size_t i{}; i < sphereNum; i++) {
        data.emplace_back(Vector<3>{0, 0, centrePos}, currentRadius);
        centrePos += 2*currentRadius + radiusDelta - spherePenetration;
        currentRadius += radiusDelta;
    }

    {
        double beg = data.front().position[2] - data.front().radius;
        double end = data.back().position[2] + data.back().radius;
        double origin = (beg + end)/2;
        Vector<3> translation{0, 0, -origin};
        std::vector<SphereData> newData;
        newData.reserve(data.size());
        for (const auto &dataElem : data)
            newData.emplace_back(dataElem.position + translation, dataElem.radius);
        std::swap(data, newData);
    }

    double volume = PolysphereWedgeTraits::calculateVolume(data, spherePenetration);
    PolysphereShape shape(data, {0, 0, 1}, std::nullopt, {0, 0, 0}, volume);
    shape.addCustomNamedPoints({{"beg", data.front().position}, {"end", data.back().position}});
    if (spherePenetration == 0)
        shape.addCustomNamedPoints({{"cm", shape.calculateMassCentre()}});
    return shape;
}

double PolysphereWedgeTraits::calculateVolume(const std::vector<SphereData> &sphereData, double spherePenetration) {
    auto volumeAccumulator = [](double volume, const SphereData &data) {
        return volume + 4./3*M_PI*std::pow(data.radius, 3);
    };
    double baseVolume = std::accumulate(sphereData.begin(), sphereData.end(), 0.0, volumeAccumulator);

    // Subtract overlapping parts between spheres
    if (spherePenetration > 0) {
        for (std::size_t i{}; i < sphereData.size() - 1; i++) {
            const auto &data1 = sphereData[i];
            const auto &data2 = sphereData[i + 1];
            double distance = data2.position[2] - data1.position[2];
            baseVolume -= VolumeCalculator::sphereIntersection(data1.radius, data2.radius, distance);
        }
    }

    return baseVolume;
}
