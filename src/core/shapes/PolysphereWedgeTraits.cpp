//
// Created by pkua on 09.03.2022.
//

#include "PolysphereWedgeTraits.h"
#include "utils/Assertions.h"


namespace legacy {
    PolysphereWedgeTraits::PolysphereGeometry
    PolysphereWedgeTraits::generateGeometry(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                                            double spherePenetration)
    {
        Expects(sphereNum >= 2);
        Expects(smallSphereRadius > 0);
        Expects(largeSphereRadius > 0);
        Expects(spherePenetration < 2*std::min(smallSphereRadius, largeSphereRadius));

        std::vector<SphereData> data;

        double centrePos{};
        double radiusDelta = (largeSphereRadius - smallSphereRadius)/static_cast<double>(sphereNum - 1);
        double currentRadius = smallSphereRadius;
        for (std::size_t i{}; i < sphereNum; i++) {
            data.emplace_back(Vector<3>{centrePos, 0, 0}, currentRadius);
            centrePos += 2*currentRadius + radiusDelta - spherePenetration;
            currentRadius += radiusDelta;
        }

        PolysphereGeometry geometry(std::move(data), {1, 0, 0}, {0, 1, 0}, {0, 0, 0});
        geometry.normalizeMassCentre();
        double end1 = geometry.getSphereData().front().position[0];
        double end2 = geometry.getSphereData().back().position[0];
        geometry.setGeometricOrigin({(end2 + largeSphereRadius + end1 - smallSphereRadius)/2, 0, 0});
        geometry.addCustomNamedPoints({{"cm", {0, 0, 0}},
                                       {"ss", geometry.getSphereData().front().position},
                                       {"sl", geometry.getSphereData().back().position}});
        return geometry;
    }
}


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
    if (spherePenetration == 0)
        geometry.addCustomNamedPoints({{"cm", geometry.calculateMassCentre()}});
    return geometry;
}
