//
// Created by Piotr Kubala on 22/12/2020.
//

#include "PolysphereBananaTraits.h"
#include "utils/Assertions.h"
#include "geometry/VolumeCalculator.h"


namespace legacy {
    PolysphereBananaTraits::PolysphereGeometry
    PolysphereBananaTraits::generateGeometry(double arcRadius, double arcAngle, std::size_t sphereNum,
                                             double sphereRadius)
    {
        Expects(arcRadius > 0);
        Expects(arcAngle > 0);
        Expects(sphereNum >= 2);
        Expects(sphereRadius > 0);

        double angleFactor = arcAngle/static_cast<double>(sphereNum - 1);
        int start = -static_cast<int>(sphereNum)/2;
        int end = static_cast<int>((sphereNum % 2 == 0) ? (sphereNum/2 - 1) : sphereNum/2);
        std::vector<SphereData> sphereData;
        sphereData.reserve(sphereNum);
        for (int i = start; i <= end; i++) {
            Vector<3> pos = Matrix<3, 3>::rotation(0, 0, -i*angleFactor) * Vector<3>{-arcRadius, 0, 0};
            pos += Vector<3>{arcRadius, 0, 0};
            sphereData.emplace_back(pos, sphereRadius);
        }

        // Calculate volume disregarding sphere overlaps - behaviour consistent with simulations pre version 0.2
        double volume = static_cast<double>(sphereNum) * 4./3*M_PI * std::pow(sphereRadius, 3);
        PolysphereGeometry geometry(std::move(sphereData), {0, 1, 0}, {-1, 0, 0}, {0, 0, 0}, volume);
        geometry.normalizeMassCentre();
        geometry.setGeometricOrigin({0, 0, 0});
        const auto &newSphereData = geometry.getSphereData();
        geometry.addCustomNamedPoints({{"cm", {0, 0, 0}},
                                       {"beg", newSphereData.front().position},
                                       {"end", newSphereData.back().position}});
        return geometry;
    }
}


PolysphereBananaTraits::PolysphereGeometry
PolysphereBananaTraits::generateGeometry(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius)
{
    Expects(arcRadius > 0);
    Expects(arcAngle > 0);
    Expects(arcAngle < 2*M_PI);
    Expects(sphereNum >= 2);
    Expects(sphereRadius > 0);
    // Reason: volume calculation is not implemented for sphereRadius > argRadius
    Expects(sphereRadius <= arcRadius);

    double angleStep = arcAngle/static_cast<double>(sphereNum - 1);
    std::vector<Vector<3>> spherePos;
    spherePos.reserve(sphereNum);
    double angle = -arcAngle/2;
    for (std::size_t i{}; i < sphereNum; i++) {
        Vector<3> pos = Matrix<3, 3>::rotation(0, angle, 0) * Vector<3>{-arcRadius, 0, 0};
        spherePos.push_back(pos);
        angle += angleStep;
    }

    if (arcAngle < M_PI) {
        Vector<3> translation = {-spherePos.front()[0], 0, 0};
        for (auto &pos : spherePos)
            pos += translation;
    }

    std::vector<PolysphereTraits::SphereData> sphereData;
    sphereData.reserve(sphereNum);
    for (const auto &pos : spherePos)
        sphereData.emplace_back(pos, sphereRadius);

    double volume = PolysphereBananaTraits::calculateVolume(sphereData, arcAngle);
    PolysphereGeometry geometry(std::move(sphereData), {0, 0, 1}, {-1, 0, 0}, {0, 0, 0}, volume);
    geometry.addCustomNamedPoints({{"beg", spherePos.front()}, {"end", spherePos.back()}});
    PolysphereBananaTraits::addMassCentre(geometry);
    return geometry;
}

void PolysphereBananaTraits::addMassCentre(PolysphereTraits::PolysphereGeometry &geometry) {
    const auto &sphereData = geometry.getSphereData();
    const auto &p1 = sphereData[0].position;
    const auto &p2 = sphereData[1].position;
    double r = sphereData[0].radius;
    double dist2 = (p2 - p1).norm2();
    constexpr double EPSILON = 1e-12;
    if (dist2 + EPSILON*EPSILON >= r*r)
        geometry.addCustomNamedPoints({{"cm", geometry.calculateMassCentre()}});
}

double PolysphereBananaTraits::calculateVolume(const std::vector<SphereData> &sphereData, double arcAngle) {
    std::size_t numSpheres = sphereData.size();
    double sphereRadius = sphereData.front().radius;
    double baseVolume = static_cast<double>(numSpheres) * 4./3*M_PI * std::pow(sphereRadius, 3);

    // Subtract overlapping parts between all spheres from first to last
    const auto &data1 = sphereData[0];
    const auto &data2 = sphereData[1];
    double distance = (data1.position - data2.position).norm();
    if (distance < data1.radius + data2.radius) {
        double overlapVolume = VolumeCalculator::sphereIntersection(data1.radius, data2.radius, distance);
        baseVolume -= static_cast<double>(numSpheres - 1) * overlapVolume;
    }

    // Subtract overlapping part between first and last sphere if they touch
    if (arcAngle > M_PI && numSpheres > 2) {
        const auto &data3 = sphereData.back();
        distance = (data1.position - data3.position).norm();
        if (distance < data1.radius + data3.radius)
            baseVolume -= VolumeCalculator::sphereIntersection(data1.radius, data3.radius, distance);
    }

    return baseVolume;
}
