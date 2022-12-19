//
// Created by Piotr Kubala on 02/01/2021.
//

#include "KMerTraits.h"
#include "utils/Assertions.h"
#include "geometry/VolumeCalculator.h"


KMerTraits::PolysphereGeometry KMerTraits::generateGeometry(std::size_t sphereNum, double sphereRadius, double distance)
{
    Expects(sphereNum >= 2);
    Expects(sphereRadius > 0);
    Expects(distance > 0);

    std::vector<SphereData> data;
    data.reserve(sphereNum);
    double sphereZ = -static_cast<double>(sphereNum - 1) / 2 * distance;
    for (std::size_t i{}; i < sphereNum; i++) {
        data.emplace_back(SphereData({0, 0, sphereZ}, sphereRadius));
        sphereZ += distance;
    }
    double volume = KMerTraits::caluclateVolume(sphereNum, sphereRadius, distance);
    PolysphereGeometry geometry(std::move(data), {0, 0, 1}, {1, 0, 0}, {0, 0, 0}, volume);
    geometry.normalizeMassCentre();
    geometry.setGeometricOrigin({0, 0, 0});
    const auto &newSphereData = geometry.getSphereData();
    geometry.addCustomNamedPoints({{"cm", {0, 0, 0}},
                                   {"beg", newSphereData.front().position},
                                   {"end", newSphereData.back().position}});
    return geometry;
}

double KMerTraits::caluclateVolume(std::size_t sphereNum, double sphereRadius, double distance) {
    double sphereVolume = 4./3*M_PI*sphereRadius*sphereRadius*sphereRadius;
    double baseVolume = static_cast<double>(sphereNum) * sphereVolume;

    if (distance < 2*sphereRadius) {
        double capHeight = sphereRadius - distance/2;
        baseVolume -= 2 * static_cast<double>(sphereNum - 1) * VolumeCalculator::sphericalCap(sphereRadius, capHeight);
    }

    return baseVolume;
}
