//
// Created by pkua on 09.03.2022.
//

#include "PolysphereWedgeTraits.h"
#include "utils/Assertions.h"


PolysphereWedgeTraits::PolysphereGeometry
PolysphereWedgeTraits::generateGeometry(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
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

    PolysphereGeometry geometry(data, {0, 0, 1}, {1, 0, 0}, {0, 0, 0});
    geometry.addCustomNamedPoints({{"beg", data.front().position}, {"end", data.back().position}});
    return geometry;
}
