//
// Created by pkua on 01.03.2022.
//

#include "PolysphereLollipopTraits.h"

#include "utils/Assertions.h"


PolysphereLollipopTraits::PolysphereGeometry
PolysphereLollipopTraits::generateGeometry(std::size_t sphereNum, double smallSphereRadius,
                                           double largeSphereRadius, double smallSpherePenetration,
                                           double largeSpherePenetration)
{
    Expects(sphereNum >= 2);
    Expects(smallSphereRadius > 0);
    Expects(largeSphereRadius > 0);
    Expects(smallSpherePenetration < 2*smallSphereRadius);
    Expects(largeSpherePenetration < 2*std::min(smallSphereRadius, largeSphereRadius));

    std::vector<SphereData> data;

    double centrePos{};
    for (std::size_t i{}; i < sphereNum - 1; i++) {
        data.emplace_back(Vector<3>{0, 0, centrePos}, smallSphereRadius);
        if (i < sphereNum - 2)
            centrePos += 2*smallSphereRadius - smallSpherePenetration;
    }

    centrePos += smallSphereRadius + largeSphereRadius - largeSpherePenetration;
    data.emplace_back(Vector<3>{0, 0, centrePos}, largeSphereRadius);

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

    PolysphereGeometry geometry(data, {0, 0, 1}, {1, 0, 0});
    geometry.addCustomNamedPoints({{"ss", data.front().position}, {"sl", data.back().position}});
    return geometry;
}

PolysphereLollipopTraits::PolysphereGeometry
PolysphereLollipopTraits::generateGeometry([[maybe_unused]] LegacyTag<0, 0, 0> tag, std::size_t sphereNum,
                                           double smallSphereRadius, double largeSphereRadius,
                                           double smallSpherePenetration, double largeSpherePenetration)
{
    Expects(sphereNum >= 2);
    Expects(smallSphereRadius > 0);
    Expects(largeSphereRadius > 0);
    Expects(smallSpherePenetration < 2*smallSphereRadius);
    Expects(largeSpherePenetration < 2*std::min(smallSphereRadius, largeSphereRadius));

    std::vector<SphereData> data;

    double centrePos{};
    for (std::size_t i{}; i < sphereNum - 1; i++) {
        data.emplace_back(Vector<3>{centrePos, 0, 0}, smallSphereRadius);
        if (i < sphereNum - 2)
            centrePos += 2*smallSphereRadius - smallSpherePenetration;
    }

    centrePos += smallSphereRadius + largeSphereRadius - largeSpherePenetration;
    data.emplace_back(Vector<3>{centrePos, 0, 0}, largeSphereRadius);

    PolysphereGeometry geometry(std::move(data), {1, 0, 0}, {0, 1, 0});
    geometry.normalizeMassCentre();
    double end1 = geometry.getSphereData().front().position[0];
    double end2 = geometry.getSphereData().back().position[0];
    geometry.setGeometricOrigin({(end2 + largeSphereRadius + end1 - smallSphereRadius)/2, 0, 0});
    const auto &newSphereData = geometry.getSphereData();
    geometry.addCustomNamedPoints({{"ss", newSphereData.front().position}, {"sl", newSphereData.back().position}});
    return geometry;
}
