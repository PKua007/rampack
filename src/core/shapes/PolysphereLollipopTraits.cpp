//
// Created by pkua on 01.03.2022.
//

#include "PolysphereLollipopTraits.h"

#include "utils/Exceptions.h"
#include "geometry/VolumeCalculator.h"


namespace legacy {
    PolysphereTraits::PolysphereShape
    PolysphereLollipopTraits::generateShape(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                                            double smallSpherePenetration, double largeSpherePenetration)
    {
        Expects(sphereNum >= 2);
        Expects(smallSphereRadius > 0);
        Expects(largeSphereRadius > 0);
        ExpectsMsg(smallSpherePenetration == 0 && largeSpherePenetration == 0,
                   "sphere penetration is disabled for version 0.1 because of incorrect volume calculation");

        std::vector<SphereData> data;

        double centrePos{};
        for (std::size_t i{}; i < sphereNum - 1; i++) {
            data.emplace_back(Vector<3>{centrePos, 0, 0}, smallSphereRadius);
            if (i < sphereNum - 2)
                centrePos += 2*smallSphereRadius - smallSpherePenetration;
        }

        centrePos += smallSphereRadius + largeSphereRadius - largeSpherePenetration;
        data.emplace_back(Vector<3>{centrePos, 0, 0}, largeSphereRadius);

        PolysphereShape shape(std::move(data), {1, 0, 0}, {0, 1, 0});
        shape.normalizeMassCentre();
        double end1 = shape.getSphereData().front().position[0];
        double end2 = shape.getSphereData().back().position[0];
        shape.setGeometricOrigin({(end2 + largeSphereRadius + end1 - smallSphereRadius) / 2, 0, 0});
        const auto &newSphereData = shape.getSphereData();
        shape.addCustomNamedPoints({{"cm", {0, 0, 0}},
                                    {"ss", newSphereData.front().position},
                                    {"sl", newSphereData.back().position}});
        return shape;
    }
}


PolysphereTraits::PolysphereShape
PolysphereLollipopTraits::generateShape(std::size_t sphereNum, double stickSphereRadius, double tipSphereRadius,
                                        double stickSpherePenetration, double tipSpherePenetration)
{
    Expects(sphereNum >= 2);
    Expects(stickSphereRadius > 0);
    Expects(tipSphereRadius > 0);
    Expects(stickSpherePenetration < 2 * stickSphereRadius);
    Expects(tipSpherePenetration < 2 * std::min(stickSphereRadius, tipSphereRadius));

    std::vector<SphereData> data;

    double centrePos{};
    for (std::size_t i{}; i < sphereNum - 1; i++) {
        data.emplace_back(Vector<3>{0, 0, centrePos}, stickSphereRadius);
        if (i < sphereNum - 2)
            centrePos += 2 * stickSphereRadius - stickSpherePenetration;
    }

    centrePos += stickSphereRadius + tipSphereRadius - tipSpherePenetration;
    data.emplace_back(Vector<3>{0, 0, centrePos}, tipSphereRadius);

    {
        double beg = data.front().position[2] - data.front().radius;
        double end = data.back().position[2] + data.back().radius;
        double origin = (beg + end) / 2;
        Vector<3> translation{0, 0, -origin};
        std::vector<SphereData> newData;
        newData.reserve(data.size());
        for (const auto &dataElem: data)
            newData.emplace_back(dataElem.position + translation, dataElem.radius);
        std::swap(data, newData);
    }

    double volume = PolysphereLollipopTraits::calculateVolume(data, stickSpherePenetration, tipSpherePenetration);
    PolysphereShape shape(data, {0, 0, 1}, std::nullopt, {0, 0, 0}, volume);
    shape.addCustomNamedPoints({{"ss", data.front().position}, {"st", data.back().position}});
    if (stickSpherePenetration == 0 && tipSpherePenetration == 0)
        shape.addCustomNamedPoints({{"cm", shape.calculateMassCentre()}});
    return shape;
}

double PolysphereLollipopTraits::calculateVolume(const std::vector<SphereData> &sphereData,
                                                 double stickSpherePenetration, double tipSpherePenetration)
{
    auto volumeAccumulator = [](double volume, const SphereData &data) {
        return volume + 4./3*M_PI*std::pow(data.radius, 3);
    };
    double baseVolume = std::accumulate(sphereData.begin(), sphereData.end(), 0.0, volumeAccumulator);

    std::size_t sphereNum = sphereData.size();

    // Subtract overlapping parts between lollipop's stick spheres
    if (stickSpherePenetration > 0) {
        double smallSphereRadius = sphereData.front().radius;
        double capVolume = VolumeCalculator::sphericalCap(smallSphereRadius, stickSpherePenetration / 2);
        baseVolume -= 2 * static_cast<double>(sphereNum - 2) * capVolume;
    }

    // Subtract overlapping parts between the last sphere in lollipop's stick and lollipop's tip sphere
    if (tipSpherePenetration > 0) {
        const auto &dataTip = sphereData[sphereNum - 1];
        const auto &dataStick = sphereData[sphereNum - 2];
        double distance = dataTip.position[2] - dataStick.position[2];
        baseVolume -= VolumeCalculator::sphereIntersection(dataTip.radius, dataStick.radius, distance);
    }

    return baseVolume;
}

