//
// Created by pkua on 09.03.2022.
//

#include "PolysphereWedgeTraits.h"
#include "utils/Assertions.h"

PolysphereWedgeTraits::PolysphereGeometry
PolysphereWedgeTraits::generateGeometry(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                                        double spherePenetration, bool normalizeMassCentre)
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
    if (normalizeMassCentre)
        geometry.normalizeMassCentre();
    double end1 = geometry.getSphereData().front().position[0];
    double end2 = geometry.getSphereData().back().position[0];
    geometry.setGeometricOrigin({(end2 + largeSphereRadius + end1 - smallSphereRadius)/2, 0, 0});
    geometry.setNamedPoints({{"ss", geometry.getSphereData().front().position},
                             {"sl", geometry.getSphereData().back().position}});
    return geometry;
}
